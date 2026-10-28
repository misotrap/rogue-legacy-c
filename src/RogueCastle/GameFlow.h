// The game loop between screens (the original RCScreenManager flow):
//   start: load save.txt, title screen; no hero yet -> heir choice
//   heir chosen -> castle (a new one, or the saved one) with the HUD
//   death -> the hero joins the family tree, save -> death screen -> heir
//   choice -> castle courtyard (left exit: manor; NPCs: blacksmith,
//   enchantress) -> castle
// The castle player's death (CurrentHealth <= 0) calls OnPlayerDeath()
// after a short pause, once the death lands.
#pragma once

#include "HUD/PlayerHUD.h"
#include "Lineage/Lineage.h"
#include "Save/SaveGame.h"
#include "Screens/LineageScreen.h"
#include "Screens/MapScreen.h"
#include "Screens/GetItemScreen.h"
#include "Screens/CreditsScreen.h"
#include "Screens/PauseScreen.h"
#include "Screens/TitleScreen.h"
#include "Screens/GameOverScreen.h"
#include "Screens/EquipmentScreen.h"
#include "Rooms/StartingRoom.h"
#include "Rooms/BossRoom.h"
#include "Types/TraitType.h"
#include "Screens/DialogueScreen.h"
#include "Screens/OptionsScreen.h"
#include "Screens/TraitEffects.h"
#include "Screens/ManorScreen.h"
#include "Screens/ProceduralLevelScreen.h"

#include <string>
#include <vector>

namespace RogueCastle
{
	class GameFlow
	{
	public:
		enum class State { Title, Lineage, Castle, Shop, GameOver, Manor, Credits };

		// savePath: e.g. "save.txt" next to the game
		bool Init(const char *assetsDir, const char *savePath);
		// One 60 Hz step; buttons = BTN_* held
		void Update(uint32_t buttons, double dt);
		void Draw(uint32_t *frame, double drawElapsed);

		void OnPlayerDeath();
		// The last boss is dead: credits, then New Game+ (EnemyObj_LastBoss.SetPlayerData)
		void OnCastleBeaten();
		void AddFamilyNode(bool beatenABoss);
		State Current() const { return m_state; }
		bool WantsQuit() const { return m_quit; }   /* "Quitter Rogue Legacy" */
		/* The Vertigo trait: the world draws upside down (the frame
		 * flips in main before the present). */
		bool VertigoActive() const
		{
			return m_level != nullptr && m_level->Player() != nullptr
			       && m_state == State::Castle
			       && m_save.hero.HasTrait(TraitType::Vertigo);
		}
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
		GetItemScreen m_getItem;
		BossRoom m_bossRoom;
		TraitEffects m_traitEffects;
		float m_clonusTimer = 0, m_shakeTimer = 0;   // Clonus
		bool m_creditsFromTitle = false;
		CreditsScreen m_credits;
		bool m_getItemOpen = false;   /* a chest reward on screen */
		PauseScreen m_pause;
		TitleScreen m_title;
		GameOverScreen m_gameOver;
		StartingRoom m_court;   /* the courtyard: rooms[0] of the castle */
		EquipmentScreen m_smith{ EquipmentScreen::Mode::Blacksmith };
		EquipmentScreen m_enchant{ EquipmentScreen::Mode::Enchantress };
		EquipmentScreen *m_shop = nullptr;
		DialogueScreen m_dialogue;   /* over the castle while open */
		bool m_architectAsking = false;   /* the open dialogue ends on the lock choice */
		bool m_tollAsking = false;        /* the open dialogue ends on the toll choice */
		OptionsScreen m_options;          /* SELECT in the pause and on the title */
		bool m_optionsOpen = false, m_quit = false;
		std::vector<uint32_t> m_lastFrame;   /* the castle frame while dying */
		bool m_paused = false;    /* Start in the castle */
		ProceduralLevelScreen *m_level = nullptr;
		State m_state = State::Lineage;
		uint32_t m_previous = 0;
		double m_deathTimer = 0;   /* the pause after the hero dies */

		void StartCastle();
		void OpenTitle();
		void DrawState(uint32_t *frame, double drawElapsed);
		void LeaveTitle();
		bool InCourtyard() const;
		void UpdateMusic();
		int MaxWeight() const;
		void Save(bool vitals = true);
	};
}
