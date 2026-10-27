// Renders the in-game HUD into a PPM (no window): ./hudtest out.ppm
#include "../src/RogueCastle/HUD/PlayerHUD.h"
#include "../src/platform.h"

#include <cstdio>

static uint32_t frame[SCREEN_W * SCREEN_H];

int main(int argc, char **argv)
{
	gfx_begin(frame);
	gfx_clear(0xFF000000u);
	gfx_fill(0, 60, SCREEN_W, 360, 0xFF241C28u);
	RogueCastle::PlayerHUD hud;
	if (!hud.LoadContent("assets")) printf("fonts missing\n");
	RogueCastle::HUDValues v;
	v.health = 73; v.maxHealth = 110; v.mana = 30; v.maxMana = 50; v.gold = 1234; v.level = 7;
	v.heroName = "Dame Dee";
	hud.Draw(frame, v);
	FILE *f = fopen(argc > 1 ? argv[1] : "hud.ppm", "wb");
	fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
	for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
	{
		unsigned char rgb[3] = { (unsigned char) (frame[i] >> 16), (unsigned char) (frame[i] >> 8), (unsigned char) frame[i] };
		fwrite(rgb, 1, 3, f);
	}
	fclose(f);
	return 0;
}
