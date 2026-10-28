// The boss rooms (BossRoomObj.cs and the four area rooms): Up at the boss
// door of an EntranceBoss room takes the hero to the linked boss room
// (PlayerObj.cs line 1910, closed once that boss is beaten). Entering the
// boss room runs the intro: the hero waits, the boss title slides in
// ("Le Guetteur / Khidr"...), then the boss song plays. Once every enemy
// of the room is dead, the area's *BossBeaten flag is saved, the boss chest
// gives its three stat bonuses (GetItem screen) and the hero is teleported
// back to the entrance room. The castle entrance's door opens once the
// four are beaten and leads, through the corridor, to the last boss, whose
// death ends the game (GameFlow then shows the credits).
#pragma once

#include "../../Text/Font.h"
#include "../Save/SaveGame.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	class ProceduralLevelScreen;
	class RoomObj;

	class BossRoom
	{
	public:
		bool LoadContent(const char *assetsDir);
		void Reset();
		// Before the level update; true while the hero's input is blocked
		bool Update(ProceduralLevelScreen *level, Save::GameState &save, uint32_t pressed, double dt, bool getItemOpen);
		void Draw(uint32_t *frame);
		// m_cutsceneRunning: the room (boss included) waits
		bool Cutscene() const { return m_phase == Phase::Intro || m_phase == Phase::Leaving; }
		// The last boss is dead: time for the ending (once)
		bool LastBossBeaten() { if (m_phase != Phase::Finished) return false; m_phase = Phase::Done; return true; }
		// The song of the boss room the hero is in ("" = silence)
		const char *Music() const;

	private:
		enum class Phase { None, Intro, Fight, Won, Chest, Leaving, Done, Ending, Finished };
		Phase m_phase = Phase::None;
		RoomObj *m_room = nullptr;
		double m_time = 0, m_fade = 0;
		std::string m_title1, m_title2;
		Text::Font m_font;

		static bool *BeatenFlag(Save::GameState &save, int levelType);
		static bool DoorOpen(RoomObj *room, Save::GameState &save);
		void Teleport(ProceduralLevelScreen *level, RoomObj *to, bool atBossDoor);
	};
}
