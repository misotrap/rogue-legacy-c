/* Software drawing into the 640x480 frame. Pixels are 0xAARRGGBB with
 * premultiplied alpha (the converted XNA textures already are).
 */
#ifndef GFX_H
#define GFX_H

#include <stddef.h>
#include <stdint.h>

typedef struct Texture
{
	int w, h;          /* the size the sprite sheets describe */
	uint32_t *px;      /* the stored texels: half size unless RL_FULLRES */
	int shift, stride; /* texel (x, y) is px[(y >> shift) * stride + (x >> shift)] */
} Texture;

/* The texel at full-size coordinates (x, y) */
static inline uint32_t gfx_texel(const Texture *t, int x, int y)
{
	return t->px[(size_t) (y >> t->shift) * t->stride + (x >> t->shift)];
}
/* The same, kept inside the source rectangle [x0, x1) x [y0, y1): at half
 * size the stored texel of an edge may be the one just outside it */
static inline uint32_t gfx_texel_in(const Texture *t, int x, int y, int x0, int y0, int x1, int y1)
{
	int s = t->shift, ix = x >> s, iy = y >> s;
	int lx = x0 >> s, hx = (x1 - 1 - s) >> s, ly = y0 >> s, hy = (y1 - 1 - s) >> s;
	if (ix > hx) ix = hx;
	if (ix < lx) ix = lx;
	if (iy > hy) iy = hy;
	if (iy < ly) iy = ly;
	return t->px[(size_t) iy * t->stride + ix];
}
/* The number of stored texels */
static inline int gfx_texel_count(const Texture *t)
{
	return t->stride * (t->h >> t->shift);
}

/* Loads a .tex file written by tools/convert_assets.py; 0 on failure */
int gfx_load_texture(Texture *t, const char *path);
void gfx_free_texture(Texture *t);

void gfx_begin(uint32_t *frame);
/* Pixels covered by the blits since the last reset (the overdraw meter) */
extern uint64_t gfx_pixels;
/* The frame passed to gfx_begin (for the text draws that run outside the
 * HUD, e.g. the level screen's floating pickup texts). */
uint32_t *gfx_frame();
void gfx_clear(uint32_t color);
/* The Vertigo trait: the world plays upside down, so the whole frame
 * flips vertically just before it is presented. */
void gfx_flip_vertical(uint32_t *frame);
void gfx_fill(int x, int y, int w, int h, uint32_t color);
/* Draws the source rectangle (sx, sy, sw, sh) of a texture with its
 * top-left corner at (dx, dy), scaled by scale (nearest texel, any
 * positive value), mirrored horizontally when flip is set. Alpha blended.
 * Rotation is not supported yet (see AGENTS.md).
 */
void gfx_blit(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip);
/* Same, multiplied by a colour 0xAARRGGBB (XNA's draw colour: TextureColor
 * and Opacity). An opaque white colour takes the plain gfx_blit path. */
void gfx_blit_tinted(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip, uint32_t colour);
/* General sprite draw (XNA SpriteBatch.Draw): the texel (originX, originY)
 * of the source rectangle lands on screen point (x, y); the sprite is
 * scaled by (scaleX, scaleY), rotated by 'rotation' radians around that
 * point, mirrored when flip is set, and tinted by colour. Falls back to the
 * fast axis-aligned blits when there is no rotation and the scale is even. */
void gfx_blit_ex(const Texture *t, int sx, int sy, int sw, int sh, float x, float y, float originX, float originY,
	float scaleX, float scaleY, float rotation, int flip, uint32_t colour);

#endif
