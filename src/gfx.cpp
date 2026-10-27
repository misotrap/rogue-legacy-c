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
