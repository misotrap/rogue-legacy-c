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
#include <memory>
#include <string>
#include <vector>

namespace DS2DEngine { class Camera2D; class GameObj; class ObjContainer; class SpriteObj; }

namespace RogueCastle
{
	class ProceduralLevelScreen;
	class RoomObj;

	class BossRoom
	{
	public:
		BossRoom();
		~BossRoom();
		bool LoadContent(const char *assetsDir);
		void Reset();
		// Before the level update; true while the hero's input is blocked
		bool Update(ProceduralLevelScreen *level, Save::GameState &save, uint32_t pressed, double dt, bool getItemOpen);
		void Draw(uint32_t *frame);
		// The intro's extra objects (orbs, blobs, the fairy's shadow), in the world
		void DrawWorld(DS2DEngine::Camera2D *camera);
		// FinalBossTalk01: the dialogue to open now (once), or nullptr
		const char *TakeDialogue() { const char *d = m_dialogue; m_dialogue = nullptr; return d; }
		// CastleEntranceRoomObj.ForceGateClosed: a run resumed at the entrance
		void ForceGateClosed() { m_gateClosed = true; m_gateTime = 2; }
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

		// The intro (each boss room's OnEnter and its tweens)
		struct CamKey { double t0, t1; float x, y; };
		std::vector<CamKey> m_camKeys;
		float m_camX0 = 0, m_camY0 = 0;
		double m_titleAt = 1, m_introEnd = 4.6, m_borders = 0;
		class EnemyObj *m_boss = nullptr;
		float m_bossScale = 1, m_bossX = 0, m_bossY = 0;
		std::vector<std::unique_ptr<DS2DEngine::SpriteObj>> m_orbs;
		std::vector<std::unique_ptr<DS2DEngine::ObjContainer>> m_blobs;
		std::unique_ptr<DS2DEngine::ObjContainer> m_shadow;
		const char *m_dialogue = nullptr;
		int m_heroClass = 0;
		// The castle entrance gate (CastleEntranceRoomObj): it drops behind
		// the hero the first time he comes in, then keeps him from going back
		DS2DEngine::SpriteObj *m_gate = nullptr;
		// The boss chest (BossRoomObj.cs line 137): it fades in and drops to
		// the floor in 4 s, then waits for the hero to open it
		std::unique_ptr<class ChestObj> m_bossChest;
		float m_chestFloor = 0;
		bool m_gateClosed = false;
		double m_gateTime = 0;
		float m_gateX = 0, m_gateY = 0;
		void UpdateGate(ProceduralLevelScreen *level, RoomObj *room, double dt);

		void StartIntro(ProceduralLevelScreen *level, Save::GameState &save);
		void UpdateIntro(ProceduralLevelScreen *level, double before);
		void EndIntro();

		static bool *BeatenFlag(Save::GameState &save, int levelType);
		static bool DoorOpen(RoomObj *room, Save::GameState &save);
		void Teleport(ProceduralLevelScreen *level, RoomObj *to, bool atBossDoor);
	};
}
