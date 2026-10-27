/* Rogue Legacy C++ port - milestone 1: a generated castle, the starting
 * room with its walls, the player walking, jumping, colliding, and room
 * transitions through the door openings (see AGENTS.md).
 */
#include "RogueCastle/Game.h"
#include "RogueCastle/Screens/ProceduralLevelScreen.h"
#include "gfx.h"
#include "platform.h"

#define TICK (1.0 / 60.0)   /* fixed game step */

int main(int argc, char **argv)
{
	const char *assets = argc > 1 ? argv[1] : "assets";
	uint32_t buttons = 0;
	double last, acc = 0.0, lastDraw;

	if (!plat_init())
	{
		fprintf(stderr, "platform init failed\n");
		return 1;
	}

	RogueCastle::ProceduralLevelScreen *level =
		new RogueCastle::ProceduralLevelScreen(assets, 0 /* time-based seed */);
	if (!level->IsValid())
	{
		fprintf(stderr, "level init failed\n");
		plat_quit();
		return 1;
	}

	last = plat_time();
	lastDraw = last;
	while (plat_poll(&buttons))
	{
		double now = plat_time();
		acc += now - last;
		last = now;
		if (acc > 0.25)
			acc = 0.25;

		/* Fixed 60 Hz logic, any number of steps per drawn frame. */
		while (acc >= TICK)
		{
			GameTime gameTime(TICK);
			RogueCastle::Game::GlobalInput.Update(buttons);
			level->Update(&gameTime);
			acc -= TICK;
		}

		double drawElapsed = now - lastDraw;
		lastDraw = now;

		gfx_begin(plat_frame());
		gfx_clear(0xFF000000u);
		level->Camera()->ElapsedTotalSeconds(drawElapsed);
		level->Draw(level->Camera());
		plat_present();
	}
	plat_quit();
	return 0;
}
