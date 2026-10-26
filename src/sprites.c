#include "sprites.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHEETS 32
#define HASH_SIZE 8192 /* power of two, > number of sprites + characters */

static Texture sheets[MAX_SHEETS];
static int nsheets;
static Sprite *sprites;
static int nsprites, capSprites;
static Character *characters;
static int ncharacters, capCharacters;
static int spriteHash[HASH_SIZE];    /* sprite index + 1, 0 = empty */
static int characterHash[HASH_SIZE];

static unsigned Hash(const char *s)
{
	unsigned h = 2166136261u;
	while (*s) h = (h ^ (unsigned char) *s++) * 16777619u;
	return h;
}

static int Lookup(const int *table, const char *name, int isCharacter)
{
	unsigned i = Hash(name) & (HASH_SIZE - 1);
	while (table[i])
	{
		const char *n = isCharacter ? characters[table[i] - 1].name : sprites[table[i] - 1].name;
		if (strcmp(n, name) == 0) return table[i] - 1;
		i = (i + 1) & (HASH_SIZE - 1);
	}
	return -1;
}

static void Insert(int *table, const char *name, int index)
{
	unsigned i = Hash(name) & (HASH_SIZE - 1);
	while (table[i]) i = (i + 1) & (HASH_SIZE - 1);
	table[i] = index + 1;
}

static Sprite *GetOrAddSprite(const char *name, int sheet)
{
	int i = Lookup(spriteHash, name, 0);
	if (i >= 0) return &sprites[i];
	if (nsprites == capSprites)
	{
		capSprites = capSprites ? capSprites * 2 : 1024;
		sprites = (Sprite*) realloc(sprites, sizeof(Sprite) * capSprites);
	}
	memset(&sprites[nsprites], 0, sizeof(Sprite));
	snprintf(sprites[nsprites].name, sizeof(sprites[nsprites].name), "%s", name);
	sprites[nsprites].sheet = sheet;
	Insert(spriteHash, name, nsprites);
	return &sprites[nsprites++];
}

static Character *GetOrAddCharacter(const char *name)
{
	int i = Lookup(characterHash, name, 1);
	if (i >= 0) return &characters[i];
	if (ncharacters == capCharacters)
	{
		capCharacters = capCharacters ? capCharacters * 2 : 256;
		characters = (Character*) realloc(characters, sizeof(Character) * capCharacters);
	}
	memset(&characters[ncharacters], 0, sizeof(Character));
	snprintf(characters[ncharacters].name, sizeof(characters[ncharacters].name), "%s", name);
	Insert(characterHash, name, ncharacters);
	return &characters[ncharacters++];
}

int spr_load_sheet(const char *dir, const char *sheet)
{
	char path[512], line[512];
	FILE *f;
	int index;
	if (nsheets >= MAX_SHEETS) return -1;
	snprintf(path, sizeof(path), "%s/%s.tex", dir, sheet);
	if (!gfx_load_texture(&sheets[nsheets], path))
	{
		fprintf(stderr, "cannot load %s\n", path);
		return -1;
	}
	index = nsheets++;
	snprintf(path, sizeof(path), "%s/%s.spr", dir, sheet);
	f = fopen(path, "r");
	if (f == NULL) return index;
	while (fgets(line, sizeof(line), f))
	{
		char name[64], other[64];
		int a, b, c, d, e, g, h;
		float fx, fy, fr;
		if (line[0] == 'I' && sscanf(line, "I\t%63s\t%d\t%d\t%d\t%d\t%d\t%d\t%d", name, &a, &b, &c, &d, &e, &g, &h) == 8)
		{
			Sprite *s = GetOrAddSprite(name, index);
			if (a >= 0 && a < MAX_FRAMES)
			{
				Image *im = &s->images[a];
				im->x = b; im->y = c; im->w = d; im->h = e; im->ax = g; im->ay = h;
				if (a + 1 > s->nimages) s->nimages = a + 1;
			}
		}
		else if (line[0] == 'A' && sscanf(line, "A\t%63s\t%d\t%d\t%d", name, &a, &b, &c) == 4)
		{
			/* SpriteLibrary.cs: frames go into an ordered list; the XML
			 * 'frame' attribute is only a label, the order of appearance is
			 * what animates. 'index' picks the frame image. */
			Sprite *s = GetOrAddSprite(name, index);
			if (s->nframes < MAX_FRAMES)
			{
				s->frames[s->nframes].image = b;
				s->frames[s->nframes].duration = c > 0 ? c : 1;
				s->lastFrame = s->nframes;
				s->nframes += 1;
			}
		}
		else if (line[0] == 'R' && sscanf(line, "R\t%63s\t%d\t%d\t%d\t%d\t%d\t%f\t%d", name, &a, &b, &c, &d, &e, &fr, &g) == 8)
		{
			/* The converter emits an R right after its A line; the original
			 * attaches hitboxes to the last frame data added (SpriteLibrary
			 * AddHitboxToLastFrameData). */
			Sprite *s = GetOrAddSprite(name, index);
			if (s->lastFrame >= 0 && s->frames[s->lastFrame].nrects < MAX_RECTS)
			{
				Hitbox *r = &s->frames[s->lastFrame].rects[s->frames[s->lastFrame].nrects++];
				r->x = b; r->y = c; r->w = d; r->h = e; r->rotation = fr; r->type = g;
			}
		}
		else if (line[0] == 'C' && sscanf(line, "C\t%63s\t%63s\t%f\t%f\t%f", name, other, &fx, &fy, &fr) == 5)
		{
			Character *ch = GetOrAddCharacter(name);
			if (ch->nparts < MAX_PARTS)
			{
				Part *p = &ch->parts[ch->nparts++];
				p->sprite = -1;
				p->x = fx; p->y = fy; p->rotation = fr;
				/* Resolved by name later: the child may be in another sheet */
				p->sprite = -(2 + (int) (GetOrAddSprite(other, index) - sprites));
			}
		}
	}
	fclose(f);
	return index;
}

int spr_find(const char *name)
{
	return Lookup(spriteHash, name, 0);
}

int spr_find_character(const char *name)
{
	return Lookup(characterHash, name, 1);
}

const Sprite *spr_get(int i)
{
	return (i >= 0 && i < nsprites) ? &sprites[i] : NULL;
}

int spr_frame_at(int sprite, double ticks)
{
	const Sprite *s = spr_get(sprite);
	int total = 0, i, t;
	if (s == NULL || s->nframes == 0) return 0;
	/* SpriteObj.cs: a frame with duration N is shown for N animation ticks
	 * (each tick one AnimationDelay long), then the animation loops. */
	for (i = 0; i < s->nframes; i += 1) total += s->frames[i].duration > 0 ? s->frames[i].duration : 1;
	t = ((int) ticks) % total;
	if (t < 0) t += total;
	for (i = 0; i < s->nframes; i += 1)
	{
		t -= s->frames[i].duration > 0 ? s->frames[i].duration : 1;
		if (t < 0) return i;
	}
	return s->nframes - 1;
}

const Character *spr_get_character(int i)
{
	Character *c;
	int p;
	if (i < 0 || i >= ncharacters) return NULL;
	c = &characters[i];
	for (p = 0; p < c->nparts; p += 1)
	{
		if (c->parts[p].sprite <= -2) c->parts[p].sprite = -(c->parts[p].sprite + 2);
	}
	return c;
}

const Texture *spr_texture(int sheet)
{
	return (sheet >= 0 && sheet < nsheets) ? &sheets[sheet] : NULL;
}

void spr_draw(int sprite, int frame, float x, float y, float scale, int flip)
{
	const Sprite *s = spr_get(sprite);
	const Image *im;
	float left;
	if (s == NULL || s->nframes == 0) return;
	frame %= s->nframes;
	if (s->frames[frame].image >= s->nimages) return;
	im = &s->images[s->frames[frame].image];
	/* The anchor is the image point placed at the sprite origin (XNA's
	 * draw origin); mirroring happens around the origin */
	left = flip ? x + (im->ax - im->w) * scale : x - im->ax * scale;
	gfx_blit(spr_texture(s->sheet), im->x, im->y, im->w, im->h, left, y - im->ay * scale, scale, flip);
}
