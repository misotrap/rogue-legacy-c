/* Rogue Legacy C++ port - milestone 0: the player character walking and
 * jumping on a flat floor (see AGENTS.md). The game code lives in
 * src/RogueCastle and src/DS2DEngine, ported class by class from the C#
 * source; this file only wires the loop together.
 */
#include "DS2DEngine/Camera2D.h"
#include "DS2DEngine/Physics/PhysicsManager.h"
#include "RogueCastle/EVs/GlobalEV.h"
#include "RogueCastle/Game.h"
#include "RogueCastle/GameObjects/Player/PlayerObj.h"
#include "gfx.h"
#include "platform.h"

#define TICK (1.0 / 60.0)   /* fixed game step */

int main(int argc, char **argv)
{
	const char *assets = argc > 1 ? argv[1] : "assets";
	uint32_t buttons = 0;
	double last, acc = 0.0, lastDraw;
	const float floorY = 600.0f;   /* flat floor until rooms exist */

	if (!plat_init())
	{
		fprintf(stderr, "platform init failed\n");
		return 1;
	}
	if (!SpriteSystem::SpriteLibrary::LoadSheet(assets, "playerSpritesheetBig"))
		return 1;

	RogueCastle::PlayerObj *player = new RogueCastle::PlayerObj();
	player->X(660.0f);
	player->Y(floorY - 40.0f);   /* the terrain box bottom is Y + 40 */
	player->UpdateCollisionBoxes();

	/* ProceduralLevelScreen.cs line 196 / Game.cs line 216. */
	DS2DEngine::PhysicsManager physicsManager;
	physicsManager.SetGravity(0, -GlobalEV::GRAVITY);
	physicsManager.TerminalVelocity = 2000;
	physicsManager.AddObject(player);

	DS2DEngine::Camera2D camera;

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
			physicsManager.Update(&gameTime);

			/* Flat floor stand-in for the room collision (milestone 1):
			 * keep the terrain box bottom on the floor, like the terrain
			 * collision response does. */
			int bottom = player->TerrainBounds().Bottom();
			if (bottom >= (int) floorY)
			{
				player->Y(player->Y() - (bottom - (int) floorY));
				player->AccelerationY(0);
				player->m_isTouchingGround = true;
				player->m_doubleJumpCount = 0;
			}
			else
				player->m_isTouchingGround = false;

			player->Update(&gameTime);
			player->UpdateCollisionBoxes();
			acc -= TICK;
		}

		double drawElapsed = now - lastDraw;
		lastDraw = now;

		gfx_begin(plat_frame());
		gfx_clear(0xFF000000u);
		gfx_fill(0, 60, SCREEN_W, 360, 0xFF241C28u);
		gfx_fill(0, 60 + (int) (floorY * 0.5f), SCREEN_W, 60, 0xFF68565Cu);
		camera.ElapsedTotalSeconds(drawElapsed);
		player->Draw(&camera);
		plat_present();
	}
	plat_quit();
	return 0;
}
