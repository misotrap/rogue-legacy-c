/* Rogue Legacy C++ port - milestone 1: a generated castle, the starting
 * room with its walls, the player walking, jumping, colliding, and room
 * transitions through the door openings (see AGENTS.md).
 */
#include "RogueCastle/Game.h"
#include "RogueCastle/GameFlow.h"
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

	/* Heir choice -> castle -> death -> manor -> heir choice (GameFlow) */
	RogueCastle::GameFlow flow;
	if (!flow.Init(assets, "save.txt"))
		fprintf(stderr, "some content is missing (see assets/)\n");

	last = plat_time();
	lastDraw = last;
	while (plat_poll(&buttons) && !flow.WantsQuit())
	{
		double now = plat_time();
		acc += now - last;
		last = now;
		if (acc > 0.25)
			acc = 0.25;

		/* Fixed 60 Hz logic, any number of steps per drawn frame. */
		while (acc >= TICK)
		{
			flow.Update(buttons, TICK);
			acc -= TICK;
		}

		double drawElapsed = now - lastDraw;
		lastDraw = now;
		flow.Draw(plat_frame(), drawElapsed);
		plat_present();
	}
	plat_quit();
	return 0;
}
