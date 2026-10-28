#include "gfx.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t *fb;

int gfx_load_texture(Texture *t, const char *path)
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
	/* premultiplied: out = src + dst * (1 - a) */
	rb = ((dst & 0x00FF00FFu) * ia + 0x00800080u) >> 8 & 0x00FF00FFu;
	g = ((dst & 0x0000FF00u) * ia + 0x00008000u) >> 8 & 0x0000FF00u;
	return src + rb + g;
}

void gfx_blit(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip)
{
	int x0, y0, x1, y1, x, y;
	float inv;
	if (t == NULL || t->px == NULL || scale <= 0.0f || sw <= 0 || sh <= 0) return;
	x0 = (int) floorf(dx + 0.5f);
	y0 = (int) floorf(dy + 0.5f);
	x1 = x0 + (int) floorf(sw * scale + 0.5f);
	y1 = y0 + (int) floorf(sh * scale + 0.5f);
	inv = 1.0f / scale;
	for (y = (y0 < 0 ? 0 : y0); y < y1 && y < SCREEN_H; y += 1)
	{
		int ty = sy + (int) ((y - y0 + 0.5f) * inv);
		const uint32_t *src;
		uint32_t *dst;
		if (ty < sy || ty >= sy + sh || ty < 0 || ty >= t->h) continue;
		src = t->px + (size_t) ty * t->w;
		dst = fb + (size_t) y * SCREEN_W;
		for (x = (x0 < 0 ? 0 : x0); x < x1 && x < SCREEN_W; x += 1)
		{
			int u = (int) ((x - x0 + 0.5f) * inv);
			int tx = flip ? sx + sw - 1 - u : sx + u;
			if (tx < 0 || tx >= t->w) continue;
			dst[x] = Blend(src[tx], dst[x]);
		}
	}
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

void gfx_blit_tinted(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip, uint32_t colour)
{
	if (colour == 0xFFFFFFFFu)
	{
		gfx_blit(t, sx, sy, sw, sh, dx, dy, scale, flip);
		return;
	}
	if ((colour >> 24) == 0) return;
	int x0, y0, x1, y1, x, y;
	float inv;
	if (t == NULL || t->px == NULL || scale <= 0.0f || sw <= 0 || sh <= 0) return;
	x0 = (int) floorf(dx + 0.5f);
	y0 = (int) floorf(dy + 0.5f);
	x1 = x0 + (int) floorf(sw * scale + 0.5f);
	y1 = y0 + (int) floorf(sh * scale + 0.5f);
	inv = 1.0f / scale;
	for (y = (y0 < 0 ? 0 : y0); y < y1 && y < SCREEN_H; y += 1)
	{
		int ty = sy + (int) ((y - y0 + 0.5f) * inv);
		const uint32_t *src;
		uint32_t *dst;
		if (ty < sy || ty >= sy + sh || ty < 0 || ty >= t->h) continue;
		src = t->px + (size_t) ty * t->w;
		dst = fb + (size_t) y * SCREEN_W;
		for (x = (x0 < 0 ? 0 : x0); x < x1 && x < SCREEN_W; x += 1)
		{
			int u = (int) ((x - x0 + 0.5f) * inv);
			int tx = flip ? sx + sw - 1 - u : sx + u;
			if (tx < 0 || tx >= t->w) continue;
			dst[x] = Blend(Tint(src[tx], colour), dst[x]);
		}
	}
}

void gfx_blit_ex(const Texture *t, int sx, int sy, int sw, int sh, float x, float y, float originX, float originY,
	float scaleX, float scaleY, float rotation, int flip, uint32_t colour)
{
	if (t == NULL || t->px == NULL || sw <= 0 || sh <= 0 || scaleX <= 0 || scaleY <= 0 || (colour >> 24) == 0) return;
	if (rotation == 0.0f && scaleX == scaleY)
	{
		/* XNA: the origin is a point of the destination quad; flipping
		 * mirrors the texels inside it (as Camera2D always did) */
		gfx_blit_tinted(t, sx, sy, sw, sh, x - originX * scaleX, y - originY * scaleY, scaleX, flip, colour);
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
			uint32_t p = t->px[(size_t) ty * t->w + tx];
			dst[px] = Blend(tinted ? Tint(p, colour) : p, dst[px]);
		}
	}
}

