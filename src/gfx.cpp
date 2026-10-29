#include "gfx.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t *fb;
uint64_t gfx_pixels = 0;

int gfx_load_texture_full(Texture *t, const char *path)
{
	FILE *f = fopen(path, "rb");
	char magic[4];
	uint32_t wh[2];
	size_t n;
	memset(t, 0, sizeof(*t));
	if (f == NULL) return 0;
	if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "RLT1", 4) != 0 || fread(wh, 4, 2, f) != 2)
	{
		fclose(f);
		return 0;
	}
	n = (size_t) wh[0] * wh[1];
	t->px = (uint32_t*) malloc(n * 4);
	if (t->px == NULL || fread(t->px, 4, n, f) != n)
	{
		free(t->px);
		t->px = NULL;
		fclose(f);
		return 0;
	}
	fclose(f);
	/* The windows' magenta: the original shows its sky render target
	 * there (the BG shader); here a plain night blue, the sky's colour */
	for (size_t i = 0; i < n; i++)
		if (t->px[i] == 0xFFFF00FFu)
			t->px[i] = 0xFF041D56u;
	t->w = (int) wh[0];
	t->h = (int) wh[1];
	t->shift = 0;
	t->stride = t->w;
	return 1;
}

void gfx_halve_texture(Texture *t, const int *rects, int count, const char *name)
{
	/* The world is always drawn at half size or less: keep one texel in
	 * four, those the 0.5 blits sample (the odd ones from each frame's own
	 * corner). Four times less memory and memory traffic on the console.
	 * The courtyard's sheet keeps its full size: its trees show at 1:1. */
	if (t->px == NULL || t->shift != 0 || t->w < 2 || t->h < 2) return;
	if (getenv("RL_FULLRES") != NULL || (name != NULL && strstr(name, "startingRoomSpritesheet") != NULL)) return;
	int hw = t->w / 2, hh = t->h / 2;
	uint32_t *half = (uint32_t *) malloc((size_t) hw * hh * 4);
	if (half == NULL) return;
	for (int y = 0; y < hh; y++)
		for (int x = 0; x < hw; x++)
			half[(size_t) y * hw + x] = t->px[(size_t) (2 * y + 1) * t->w + 2 * x + 1];
	/* A frame starting on an odd column (row) is sampled on its even full
	 * texels: otherwise its picture moves a pixel against the frames on
	 * even ones and the animations wobble */
	for (int i = 0; i < count; i++)
	{
		int fx = rects[4 * i], fy = rects[4 * i + 1], fw = rects[4 * i + 2], fh = rects[4 * i + 3];
		if (((fx | fy) & 1) == 0 || fw <= 0 || fh <= 0) continue;
		int px = (fx & 1) ? 0 : 1, py = (fy & 1) ? 0 : 1;
		for (int iy = fy >> 1; iy <= (fy + fh - 1) >> 1 && iy < hh; iy++)
			for (int ix = fx >> 1; ix <= (fx + fw - 1) >> 1 && ix < hw; ix++)
			{
				int sx = 2 * ix + px, sy = 2 * iy + py;
				if (sx < fx) sx = fx;
				if (sx > fx + fw - 1) sx = fx + fw - 1;
				if (sy < fy) sy = fy;
				if (sy > fy + fh - 1) sy = fy + fh - 1;
				half[(size_t) iy * hw + ix] = t->px[(size_t) sy * t->w + sx];
			}
	}
	free(t->px);
	t->px = half;
	t->shift = 1;
	t->stride = hw;
}

int gfx_load_texture(Texture *t, const char *path)
{
	if (!gfx_load_texture_full(t, path)) return 0;
	gfx_halve_texture(t, NULL, 0, path);
	return 1;
}

void gfx_free_texture(Texture *t)
{
	free(t->px);
	t->px = NULL;
}

void gfx_begin(uint32_t *frame)
{
	fb = frame;
}

uint32_t *gfx_frame()
{
	return fb;
}

void gfx_flip_vertical(uint32_t *frame)
{
	/* The Vertigo trait: row swap, top against bottom. */
	for (int y = 0; y < SCREEN_H / 2; y++)
	{
		uint32_t *a = frame + (size_t) y * SCREEN_W;
		uint32_t *b = frame + (size_t) (SCREEN_H - 1 - y) * SCREEN_W;
		for (int x = 0; x < SCREEN_W; x++)
		{
			uint32_t t = a[x];
			a[x] = b[x];
			b[x] = t;
		}
	}
}

void gfx_clear(uint32_t color)
{
	int i;
	for (i = 0; i < SCREEN_W * SCREEN_H; i += 1) fb[i] = color;
}

void gfx_fill(int x, int y, int w, int h, uint32_t color)
{
	int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
	int x1 = x + w > SCREEN_W ? SCREEN_W : x + w, y1 = y + h > SCREEN_H ? SCREEN_H : y + h;
	int i, j;
	for (j = y0; j < y1; j += 1)
	{
		uint32_t *row = fb + (size_t) j * SCREEN_W;
		for (i = x0; i < x1; i += 1) row[i] = color;
	}
}

static inline uint32_t Blend(uint32_t src, uint32_t dst)
{
	uint32_t a = src >> 24, ia = 255 - a;
	uint32_t rb, g;
	if (a == 255) return src;
	if (a == 0) return dst;
	/* premultiplied: out = src + dst * (1 - a), the alpha too (the room
	 * caches keep it: a blended texel over an opaque one is opaque) */
	rb = ((dst & 0x00FF00FFu) * ia + 0x00800080u) >> 8 & 0x00FF00FFu;
	g = (((dst >> 8) & 0x00FF00FFu) * ia + 0x00800080u) & 0xFF00FF00u;
	return src + rb + g;
}

/* A scaled blit in 16.16 fixed point (no float per pixel: the console's
 * CPU is slow at it). tint: 0 = plain, else the per-channel multipliers
 * of BlitTint (16.16) applied before blending. */
struct TintK { uint32_t a, r, g, b; };

static inline uint32_t ApplyTint(uint32_t p, const TintK &k)
{
	uint32_t a = ((p >> 24) * k.a + 32768) >> 16;
	uint32_t r = (((p >> 16) & 255) * k.r + 32768) >> 16;
	uint32_t g = (((p >> 8) & 255) * k.g + 32768) >> 16;
	uint32_t b = ((p & 255) * k.b + 32768) >> 16;
	return (a << 24) | (r << 16) | (g << 8) | b;
}

static void Blit(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, float scaleY, int flip, const TintK *tint)
{
	if (t == NULL || t->px == NULL || scale <= 0.0f || scaleY <= 0.0f || sw <= 0 || sh <= 0) return;
	int x0 = (int) floorf(dx + 0.5f), y0 = (int) floorf(dy + 0.5f);
	int x1 = x0 + (int) floorf(sw * scale + 0.5f), y1 = y0 + (int) floorf(sh * scaleY + 0.5f);
	float inv = 1.0f / scale, invY = 1.0f / scaleY;
	int32_t step = (int32_t) (inv * 65536.0f);
	int xs = x0 < 0 ? 0 : x0, xe = x1 < SCREEN_W ? x1 : SCREEN_W;
	int ys = y0 < 0 ? 0 : y0, ye = y1 < SCREEN_H ? y1 : SCREEN_H;
	if (xs >= xe || ys >= ye) return;
	/* The texels this blit may read: [sx, sx + sw) inside the texture */
	int lo = sx < 0 ? 0 : sx, hi = sx + sw < t->w ? sx + sw : t->w;
	int32_t u0 = (int32_t) ((xs - x0 + 0.5f) * inv * 65536.0f);
	/* At half size, the stored texel of a frame's edge may be the one
	 * just outside it: the stored rows and columns are kept inside */
	const int shift = t->shift;
	const int loIdx = lo >> shift, hiIdx = (hi - 1 - shift) >> shift;
	const int loRow = sy >> shift, hiRow = (sy + sh - 1 - shift) >> shift;
	/* The stored column of each destination column, worked out once per
	 * blit (the per-pixel work is then one table read): the columns are
	 * monotonic, so those outside the frame are only at the two ends */
	static int cols[SCREEN_W];
	int first = xs, last = xe;   /* the drawn columns: [first, last) */
	{
		int32_t u = u0;
		int n = 0;
		for (int x = xs; x < xe; x++, u += step)
		{
			int iu = u >> 16;
			int tx = flip ? sx + sw - 1 - iu : sx + iu;
			if (tx < lo || tx >= hi)
			{
				if (n == 0) first = x + 1;
				else if (last == xe) last = x;
				continue;
			}
			if (last != xe) continue;
			int ix = tx >> shift;
			if (ix > hiIdx) ix = hiIdx;
			if (ix < loIdx) ix = loIdx;
			cols[n++] = ix;
		}
		if (first >= last) return;
	}
	const int count = last - first;
	for (int y = ys; y < ye; y++)
	{
		int ty = sy + (int) ((y - y0 + 0.5f) * invY);
		if (ty < sy || ty >= sy + sh || ty < 0 || ty >= t->h) continue;
		int iy = ty >> shift;
		if (iy > hiRow) iy = hiRow;
		if (iy < loRow) iy = loRow;
		const uint32_t *src = t->px + (size_t) iy * t->stride;
		uint32_t *dst = fb + (size_t) y * SCREEN_W + first;
		gfx_pixels += (uint64_t) count;
		if (tint == NULL)
		{
			for (int i = 0; i < count; i++)
			{
				uint32_t p = src[cols[i]];
				uint32_t a = p >> 24;
				if (a == 255) dst[i] = p;
				else if (a != 0) dst[i] = Blend(p, dst[i]);
			}
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				uint32_t p = ApplyTint(src[cols[i]], *tint);
				uint32_t a = p >> 24;
				if (a == 255) dst[i] = p;
				else if (a != 0) dst[i] = Blend(p, dst[i]);
			}
		}
	}
}

void gfx_blit(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip)
{
	Blit(t, sx, sy, sw, sh, dx, dy, scale, scale, flip, NULL);
}

void gfx_blit_tinted(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip, uint32_t colour)
{
	if (colour == 0xFFFFFFFFu)
	{
		Blit(t, sx, sy, sw, sh, dx, dy, scale, scale, flip, NULL);
		return;
	}
	uint32_t ca = colour >> 24;
	if (ca == 0) return;
	/* Premultiplied texel times the colour, whose alpha scales everything:
	 * channel * c * ca / 255^2, as 16.16 multipliers */
	TintK k;
	k.a = (ca * 65536u + 127) / 255;
	k.r = (((colour >> 16) & 255) * ca * 65536u + 32512) / 65025;
	k.g = (((colour >> 8) & 255) * ca * 65536u + 32512) / 65025;
	k.b = ((colour & 255) * ca * 65536u + 32512) / 65025;
	Blit(t, sx, sy, sw, sh, dx, dy, scale, scale, flip, &k);
}

/* Premultiplied texel times a colour (whose alpha scales everything) */
static inline uint32_t Tint(uint32_t p, uint32_t c)
{
	uint32_t ca = c >> 24;
	uint32_t a = ((p >> 24) * ca + 127) / 255;
	uint32_t r = ((((p >> 16) & 255) * ((c >> 16) & 255) + 127) / 255 * ca + 127) / 255;
	uint32_t g = ((((p >> 8) & 255) * ((c >> 8) & 255) + 127) / 255 * ca + 127) / 255;
	uint32_t b = (((p & 255) * (c & 255) + 127) / 255 * ca + 127) / 255;
	return (a << 24) | (r << 16) | (g << 8) | b;
}

void gfx_blit_ex(const Texture *t, int sx, int sy, int sw, int sh, float x, float y, float originX, float originY,
	float scaleX, float scaleY, float rotation, int flip, uint32_t colour)
{
	if (t == NULL || t->px == NULL || sw <= 0 || sh <= 0 || scaleX <= 0 || scaleY <= 0 || (colour >> 24) == 0) return;
	if (rotation == 0.0f)
	{
		/* XNA: the origin is a point of the destination quad; flipping
		 * mirrors the texels inside it (as Camera2D always did). Any
		 * axis-aligned scale takes the fixed-point path. */
		if (scaleX == scaleY)
		{
			gfx_blit_tinted(t, sx, sy, sw, sh, x - originX * scaleX, y - originY * scaleY, scaleX, flip, colour);
			return;
		}
		if (colour == 0xFFFFFFFFu)
		{
			Blit(t, sx, sy, sw, sh, x - originX * scaleX, y - originY * scaleY, scaleX, scaleY, flip, NULL);
			return;
		}
		uint32_t ca = colour >> 24;
		TintK k;
		k.a = (ca * 65536u + 127) / 255;
		k.r = (((colour >> 16) & 255) * ca * 65536u + 32512) / 65025;
		k.g = (((colour >> 8) & 255) * ca * 65536u + 32512) / 65025;
		k.b = ((colour & 255) * ca * 65536u + 32512) / 65025;
		Blit(t, sx, sy, sw, sh, x - originX * scaleX, y - originY * scaleY, scaleX, scaleY, flip, &k);
		return;
	}
	float c = cosf(rotation), s = sinf(rotation);
	/* Screen bounding box of the four transformed corners */
	float cx[4] = { -originX, sw - originX, -originX, sw - originX };
	float cy[4] = { -originY, -originY, sh - originY, sh - originY };
	float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
	for (int i = 0; i < 4; i++)
	{
		float px = cx[i] * scaleX, py = cy[i] * scaleY;
		float rx = x + px * c - py * s, ry = y + px * s + py * c;
		if (rx < minX) minX = rx;
		if (rx > maxX) maxX = rx;
		if (ry < minY) minY = ry;
		if (ry > maxY) maxY = ry;
	}
	int x0 = (int) floorf(minX), x1 = (int) ceilf(maxX), y0 = (int) floorf(minY), y1 = (int) ceilf(maxY);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > SCREEN_W) x1 = SCREEN_W;
	if (y1 > SCREEN_H) y1 = SCREEN_H;
	int tinted = colour != 0xFFFFFFFFu;
	for (int py = y0; py < y1; py++)
	{
		uint32_t *dst = fb + (size_t) py * SCREEN_W;
		for (int px = x0; px < x1; px++)
		{
			/* Inverse transform of the pixel centre into the source rectangle */
			float dx = px + 0.5f - x, dy = py + 0.5f - y;
			float u = (dx * c + dy * s) / scaleX + originX;
			float v = (-dx * s + dy * c) / scaleY + originY;
			if (u < 0 || v < 0 || u >= sw || v >= sh) continue;
			int tx = flip ? sx + sw - 1 - (int) u : sx + (int) u;
			int ty = sy + (int) v;
			if (tx < 0 || ty < 0 || tx >= t->w || ty >= t->h) continue;
			uint32_t p = gfx_texel(t, tx, ty);
			dst[px] = Blend(tinted ? Tint(p, colour) : p, dst[px]);
		}
	}
}

