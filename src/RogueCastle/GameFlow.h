// The game loop between screens (the original RCScreenManager flow):
//   start: load save.txt; no hero yet -> heir choice
//   heir chosen -> castle (a new one, or the saved one) with the HUD
//   death -> the hero joins the family tree, save -> manor -> heir choice
// The castle screen calls OnPlayerDeath() when the hero dies. Until the
// player has health, SELECT + START together simulates a death.
#pragma once

#include "HUD/PlayerHUD.h"
#include "Lineage/Lineage.h"
#include "Save/SaveGame.h"
#include "Screens/LineageScreen.h"
#include "Screens/ManorScreen.h"
#include "Screens/ProceduralLevelScreen.h"

#include <string>

namespace RogueCastle
{
	class GameFlow
	{
	public:
		enum class State { Lineage, Castle, Manor };

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
		ProceduralLevelScreen *m_level = nullptr;
		State m_state = State::Lineage;
		uint32_t m_previous = 0;

		void StartCastle();
		void Save();
	};
}
