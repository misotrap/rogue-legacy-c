/* Sprite library: frame images, animations, hitboxes and multi-part
 * characters, read from the .spr files (see tools/convert_assets.py).
 * Coordinates are in original texture pixels.
 */
#ifndef SPRITES_H
#define SPRITES_H

#include "gfx.h"

#define MAX_FRAMES 64
#define MAX_PARTS 24
#define MAX_RECTS 4

typedef struct Image
{
	short x, y, w, h;   /* in the sheet */
	short ax, ay;       /* anchor: drawing offset is -anchor */
} Image;

typedef struct Hitbox
{
	short x, y, w, h;   /* relative to the frame's top-left */
	short type;         /* see AGENTS.md: types come from SpriteSystem */
	float rotation;
} Hitbox;

typedef struct Frame
{
	short image;        /* index into Sprite.images */
	short duration;     /* in animation ticks */
	short nrects;
	Hitbox rects[MAX_RECTS];
} Frame;

typedef struct Sprite
{
	char name[64];
	int sheet;          /* index into the loaded sheet textures */
	int nimages, nframes;
	Image images[MAX_FRAMES];
	Frame frames[MAX_FRAMES];
} Sprite;

typedef struct Part
{
	int sprite;         /* index into the sprite table */
	float x, y, rotation;
} Part;

typedef struct Character
{
	char name[64];
	int nparts;
	Part parts[MAX_PARTS];
} Character;

/* Loads assets/<sheet>.tex and .spr; returns the sheet index or -1 */
int spr_load_sheet(const char *dir, const char *sheet);
int spr_find(const char *spriteName);          /* -1 if unknown */
int spr_find_character(const char *name);      /* -1 if unknown */
const Sprite *spr_get(int sprite);
const Character *spr_get_character(int character);
const Texture *spr_texture(int sheet);

/* Draws frame 'frame' of a sprite whose origin (anchor point) is at screen
 * position (x, y), at the given scale */
void spr_draw(int sprite, int frame, float x, float y, float scale, int flip);

#endif
