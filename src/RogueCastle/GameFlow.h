// The game loop between screens (the original RCScreenManager flow):
//   start: load save.txt, title screen; no hero yet -> heir choice
//   heir chosen -> castle (a new one, or the saved one) with the HUD
//   death -> the hero joins the family tree, save -> death screen -> manor
//   -> heir choice
// The castle player's death (CurrentHealth <= 0) calls OnPlayerDeath()
// after a short pause, once the death lands.
#pragma once

#include "HUD/PlayerHUD.h"
#include "Lineage/Lineage.h"
#include "Save/SaveGame.h"
#include "Screens/LineageScreen.h"
#include "Screens/MapScreen.h"
#include "Screens/PauseScreen.h"
#include "Screens/TitleScreen.h"
#include "Screens/GameOverScreen.h"
#include "Screens/ManorScreen.h"
#include "Screens/ProceduralLevelScreen.h"

#include <string>
#include <vector>

namespace RogueCastle
{
	class GameFlow
	{
	public:
		enum class State { Title, Lineage, Castle, GameOver, Manor };

		// savePath: e.g. "save.txt" next to the game
		bool Init(const char *assetsDir, const char *savePath);
		// One 60 Hz step; buttons = BTN_* held
		void Update(uint32_t buttons, double dt);
		void Draw(uint32_t *frame, double drawElapsed);

		void OnPlayerDeath();
		State Current() const { return m_state; }
		Save::GameState &Game() { return m_save; }

	private:
		std::string m_assets, m_savePath;
		Save::GameState m_save;
		Lineage::Generator m_generator;
		LineageScreen m_lineage;
		ManorScreen m_manor;
		PlayerHUD m_hud;
		MapScreen m_map;
		bool m_mapOpen = false;   /* Select in the castle */
		PauseScreen m_pause;
		TitleScreen m_title;
		GameOverScreen m_gameOver;
		std::vector<uint32_t> m_lastFrame;   /* the castle frame while dying */
		bool m_paused = false;    /* Start in the castle */
		ProceduralLevelScreen *m_level = nullptr;
		State m_state = State::Lineage;
		uint32_t m_previous = 0;
		double m_deathTimer = 0;   /* the pause after the hero dies */

		void StartCastle();
		void LeaveTitle();
		void Save();
	};
}
