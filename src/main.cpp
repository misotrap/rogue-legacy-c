/* Rogue Legacy C++ port - milestone 1: a generated castle, the starting
 * room with its walls, the player walking, jumping, colliding, and room
 * transitions through the door openings (see AGENTS.md).
 */
#include <cstdlib>
#include "RogueCastle/Game.h"
#include "RogueCastle/GameFlow.h"
#include "gfx.h"
#include "perf.h"
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
	/* Dev/test: RL_SAVE=<file> plays on another save (tests leave the real one alone) */
	const char *savePath = getenv("RL_SAVE") != nullptr ? getenv("RL_SAVE") : "save.txt";
	if (!flow.Init(assets, savePath))
		fprintf(stderr, "some content is missing (see assets/)\n");

	last = plat_time();
	lastDraw = last;
	while (plat_poll(&buttons) && !flow.WantsQuit())
	{
		perf_start();
		double now = plat_time();
		acc += now - last;
		last = now;
		if (acc > 0.25)
			acc = 0.25;

		/* Fixed 60 Hz logic, any number of steps per drawn frame. */
		int steps = 0;
		while (acc >= TICK)
		{
			flow.Update(buttons, TICK);
			acc -= TICK;
			steps++;
		}
		perf_mark(PERF_UPDATE);

		double drawElapsed = now - lastDraw;
		lastDraw = now;
		flow.Draw(plat_frame(), drawElapsed);
		/* The Vertigo trait: the whole world plays upside down. */
		if (flow.VertigoActive())
			gfx_flip_vertical(plat_frame());
		perf_draw(plat_frame());
		perf_mark(PERF_HUD);
		plat_present();
		perf_mark(PERF_PRESENT);
		perf_frame(steps);
	}
	plat_quit();
	return 0;
}
