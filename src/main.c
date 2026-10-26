/* Rogue Legacy C port - milestone 0: the player character (multi-part,
 * animated) walking and jumping on a floor. See AGENTS.md for the plan.
 */
#include "gfx.h"
#include "platform.h"
#include "sprites.h"
#include <stdio.h>
#include <string.h>

#define WORLD_SCALE 0.5f   /* 1320x720 world -> 660x360, letterboxed in 640x480 */
#define VIEW_Y 60          /* top of the letterboxed play area */
#define TICK (1.0 / 60.0)  /* fixed game step */

typedef struct Player
{
	float x, y, vx, vy;   /* world units, origin at the feet centre */
	int onGround, flip;
	int character;        /* current *_Character */
	double animTime;
} Player;

/* Parts the default knight does not show (see PlayerObj.cs for the rules) */
static int HiddenPart(const char *name)
{
	static const char *hidden[] = { "DragonWings", "Boobs", "Bowtie", "Glasses", "Spelunker", "Hair", NULL };
	int i;
	for (i = 0; hidden[i]; i += 1)
		if (strstr(name, hidden[i])) return 1;
	return 0;
}

static void DrawCharacter(int character, double time, float sx, float sy, float scale, int flip)
{
	const Character *c = spr_get_character(character);
	int i;
	if (c == NULL) return;
	for (i = 0; i < c->nparts; i += 1)
	{
		const Part *p = &c->parts[i];
		const Sprite *s = spr_get(p->sprite);
		int frame;
		if (s == NULL || s->nframes == 0 || HiddenPart(s->name)) continue;
		/* Animation speed: PlayerObj uses AnimationDelay = 1/10 s per tick */
		frame = (int) (time * 10.0) % s->nframes;
		spr_draw(p->sprite, frame, sx + (flip ? -p->x : p->x) * scale, sy + p->y * scale, scale, flip);
	}
}

int main(int argc, char **argv)
{
	const char *assets = argc > 1 ? argv[1] : "assets";
	uint32_t buttons = 0, prev = 0;
	Player pl;
	int idle, walk, jump;
	double last, acc = 0.0;
	const float floorY = 600.0f;

	if (!plat_init())
	{
		fprintf(stderr, "platform init failed\n");
		return 1;
	}
	if (spr_load_sheet(assets, "playerSpritesheetBig") < 0) return 1;
	idle = spr_find_character("PlayerIdle_Character");
	walk = spr_find_character("PlayerWalking_Character");
	jump = spr_find_character("PlayerJumping_Character");

	memset(&pl, 0, sizeof(pl));
	pl.x = 660.0f;
	pl.y = floorY;
	pl.character = idle;
	last = plat_time();

	while (plat_poll(&buttons))
	{
		double now = plat_time();
		acc += now - last;
		last = now;
		if (acc > 0.25) acc = 0.25;
		/* Fixed 60 Hz logic, any number of steps per drawn frame */
		while (acc >= TICK)
		{
			float dir = ((buttons & BTN_RIGHT) ? 1.0f : 0.0f) - ((buttons & BTN_LEFT) ? 1.0f : 0.0f);
			acc -= TICK;
			pl.vx = dir * 500.0f;          /* TODO: PlayerObj.cs speeds */
			if (dir != 0.0f) pl.flip = dir < 0.0f;
			if (pl.onGround && (buttons & BTN_B) && !(prev & BTN_B))
			{
				pl.vy = -1100.0f;          /* TODO: PlayerObj.cs jump */
				pl.onGround = 0;
			}
			pl.vy += 2400.0f * (float) TICK;
			pl.x += pl.vx * (float) TICK;
			pl.y += pl.vy * (float) TICK;
			if (pl.y >= floorY)
			{
				pl.y = floorY;
				pl.vy = 0.0f;
				pl.onGround = 1;
			}
			pl.character = !pl.onGround ? jump : (dir != 0.0f ? walk : idle);
			pl.animTime += TICK;
			prev = buttons;
		}

		gfx_begin(plat_frame());
		gfx_clear(0xFF000000u);
		gfx_fill(0, VIEW_Y, SCREEN_W, 360, 0xFF241C28u);
		gfx_fill(0, VIEW_Y + (int) (floorY * WORLD_SCALE), SCREEN_W, 360 - (int) (floorY * WORLD_SCALE), 0xFF68565Cu);
		/* The player is drawn at scale 2 in the world: 1 texel per pixel */
		DrawCharacter(pl.character, pl.animTime, pl.x * WORLD_SCALE, VIEW_Y + pl.y * WORLD_SCALE, 2.0f * WORLD_SCALE, pl.flip);
		plat_present();
	}
	plat_quit();
	return 0;
}
