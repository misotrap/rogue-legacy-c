/* Software drawing into the 640x480 frame. Pixels are 0xAARRGGBB with
 * premultiplied alpha (the converted XNA textures already are).
 */
#ifndef GFX_H
#define GFX_H

#include <stdint.h>

typedef struct Texture
{
	int w, h;
	uint32_t *px;
} Texture;

/* Loads a .tex file written by tools/convert_assets.py; 0 on failure */
int gfx_load_texture(Texture *t, const char *path);
void gfx_free_texture(Texture *t);

void gfx_begin(uint32_t *frame);
void gfx_clear(uint32_t color);
void gfx_fill(int x, int y, int w, int h, uint32_t color);
/* Draws the source rectangle (sx, sy, sw, sh) of a texture with its
 * top-left corner at (dx, dy), scaled by scale (nearest texel, any
 * positive value), mirrored horizontally when flip is set. Alpha blended.
 * Rotation is not supported yet (see AGENTS.md).
 */
void gfx_blit(const Texture *t, int sx, int sy, int sw, int sh, float dx, float dy, float scale, int flip);

#endif
