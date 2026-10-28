#include "BossRoom.h"

#include "../../Audio/Audio.h"
#include "../../Text/Loc.h"
#include "../../platform.h"
#include "../GameObjects/ChestObj.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/Player/PlayerObj.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../Screens/EnemyNames.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "../Types/ChestType.h"
#include "../Types/EnemyType.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const double TITLE_AT = 1.0, INTRO_END = 4.6;   // DisplayBossTitle, EndCutscene

		const char *EnemyLocID(const std::string &name)
		{
			for (const EnemyNameLoc &e : ENEMY_NAMES)
				if (name == e.name) return e.locID;
			return name.c_str();
		}

		float Clamp01(float t) { return t < 0 ? 0 : t > 1 ? 1 : t; }
		float QuadOut(float t) { t = Clamp01(t); return t * (2 - t); }
	}

	bool BossRoom::LoadContent(const char *assetsDir)
	{
		return m_font.Load(assetsDir, "Junicode");
	}

	void BossRoom::Reset()
	{
		m_phase = Phase::None;
		m_room = nullptr;
		m_fade = 0;
	}

	bool *BossRoom::BeatenFlag(Save::GameState &save, int levelType)
	{
		switch ((Levels::LevelType) levelType)
		{
		case Levels::LevelType::GARDEN: return &save.FairyBossBeaten;
		case Levels::LevelType::TOWER: return &save.FireballBossBeaten;
		case Levels::LevelType::DUNGEON: return &save.BlobBossBeaten;
		default: return &save.EyeballBossBeaten;
		}
	}

	const char *BossRoom::Music() const
	{
		if (m_room == nullptr || m_phase == Phase::Intro) return "";
		if (m_phase != Phase::Fight) return "TitleScreenSong";   // the win stinger
		switch (m_room->LevelType())
		{
		case Levels::LevelType::GARDEN: return "GardenBossSong";
		case Levels::LevelType::TOWER: return "TowerBossSong";
		case Levels::LevelType::DUNGEON: return "DungeonBoss";
		default: return "CastleBossSong";
		}
	}

	// TeleportPlayer / EnterBossRoom: at the bottom of the room's boss door,
	// or the middle of the room's floor
	void BossRoom::Teleport(ProceduralLevelScreen *level, RoomObj *to, bool atBossDoor)
	{
		PlayerObj *player = level->Player();
		float x = to->X() + to->Width() / 2.0f, bottom = to->Y() + to->Height() - 60;
		if (atBossDoor)
			for (const RoomObj::DoorInfo &d : to->DoorList())
				if (d.bossDoor)
				{
					x = d.x + d.width / 2.0f;
					bottom = d.y + d.height;
					break;
				}
		Rectangle b = player->Bounds();
		player->X(x);
		player->Y(bottom - (b.Bottom() - player->Y()) - 1);
		m_fade = 0.4;
	}

	bool BossRoom::Update(ProceduralLevelScreen *level, Save::GameState &save, uint32_t pressed, double dt, bool getItemOpen)
	{
		if (m_fade > 0) m_fade -= dt;
		PlayerObj *player = level->Player();
		RoomObj *room = level->CurrentRoom();
		if (player == nullptr || room == nullptr || player->IsKilled) return false;

		/* Dev/test: RL_DEBUG_BOSS=<LevelType 1-4> puts the hero at that
		 * area's boss door. */
		static const char *debugBoss = getenv("RL_DEBUG_BOSS");
		if (debugBoss != nullptr)
		{
			int type = atoi(debugBoss);
			debugBoss = nullptr;
			for (RoomObj *r : level->RoomList())
				if (r->Name() == "EntranceBoss" && (int) r->LevelType() == type)
				{
					Teleport(level, r, true);
					fprintf(stderr, "boss: at the door of area %d, boss room %d\n", type, r->LinkedRoom());
					return true;
				}
		}

		// The boss door: Up in front of it, while that boss lives
		if (room->Name() == "EntranceBoss" && (pressed & BTN_UP) && player->IsTouchingGround()
			&& room->LinkedRoom() >= 0 && room->LinkedRoom() < (int) level->RoomList().size()
			&& !*BeatenFlag(save, (int) room->LevelType()))
		{
			Rectangle b = player->Bounds();
			for (const RoomObj::DoorInfo &d : room->DoorList())
			{
				float dx = d.x, dy = d.y;
				if (d.bossDoor && b.Right() > dx && b.X < dx + d.width && b.Bottom() > dy && b.Top() < dy + d.height)
				{
					Audio::PlaySound("Boss_Door_Open");
					Teleport(level, level->RoomList()[room->LinkedRoom()], true);
					return true;
				}
			}
		}

		if (room->Name() != "Boss")
		{
			if (m_room != nullptr && room != m_room) m_room = nullptr, m_phase = Phase::None;
			return false;
		}

		if (room != m_room)
		{
			// BossRoomObj.OnEnter
			m_room = room;
			m_time = 0;
			m_phase = *BeatenFlag(save, (int) room->LevelType()) ? Phase::Done : Phase::Intro;
			if (m_phase == Phase::Intro)
			{
				Audio::StopMusic();
				const char *title = "LOC_ID_ENEMY_NAME_118";   // the Lookout (Khidr)
				switch (room->LevelType())
				{
				case Levels::LevelType::GARDEN: title = "LOC_ID_ENEMY_NAME_120"; break;
				case Levels::LevelType::TOWER: title = "LOC_ID_ENEMY_NAME_122"; break;
				case Levels::LevelType::DUNGEON: title = "LOC_ID_ENEMY_NAME_119"; break;
				default: break;
				}
				m_title1 = Text::Loc::Get(title);
				m_title2.clear();
				for (EnemyObj *e : room->EnemyList())
					if (e != nullptr && !e->NonKillable && e->Type != EnemyType::SpikeTrap) { m_title2 = Text::Loc::Get(EnemyLocID(e->Name)); break; }
			}
		}
		m_time += dt;

		switch (m_phase)
		{
		case Phase::Intro:
		{
			double before = m_time - dt;
			if (before < TITLE_AT && m_time >= TITLE_AT) Audio::PlaySound("Boss_Title");
			if (before < TITLE_AT + 3 && m_time >= TITLE_AT + 3) Audio::PlaySound("Boss_Title_Exit");
			if (m_time >= INTRO_END) m_phase = Phase::Fight;
			return true;
		}
		case Phase::Fight:
		{
			bool alive = false, any = false;
			for (EnemyObj *e : room->EnemyList())
				if (e != nullptr && !e->NonKillable && e->Type != EnemyType::SpikeTrap) { any = true; if (!e->IsKilled) alive = true; }
			/* Dev/test: RL_DEBUG_BOSS_WIN kills the boss after the intro. */
			if (alive && getenv("RL_DEBUG_BOSS_WIN") != nullptr)
				level->DamageAllEnemies(1000000);
			if (any && !alive)
			{
				// BossCleanup: the flag is saved, the boss chest falls
				*BeatenFlag(save, (int) room->LevelType()) = true;
				save.NewBossBeaten = true;
				m_phase = Phase::Won;
				if (getenv("RL_DEBUG_TRACE") != nullptr) fprintf(stderr, "boss: beaten\n");
				m_time = 0;
			}
			return false;
		}
		case Phase::Won:
			if (m_time >= 2.5)
			{
				// The boss chest (ChestType.Boss: three stat drops)
				ChestObj chest;
				chest.SetChestType(ChestType::Boss);
				chest.X(room->X() + room->Width() / 2.0f);
				chest.Y(room->Y() + room->Height() - 120);
				chest.OpenChest(level, player);
				m_phase = Phase::Chest;
				m_time = 0;
			}
			return false;
		case Phase::Chest:
			if (!getItemOpen && m_time > 0.5)
			{
				Audio::PlaySound("Teleport_Disappear");
				m_phase = Phase::Leaving;
				m_time = 0;
			}
			return false;
		case Phase::Leaving:
			if (m_time >= 0.8)
			{
				int back = room->LinkedRoom();
				if (back >= 0 && back < (int) level->RoomList().size())
					Teleport(level, level->RoomList()[back], true);
				Audio::PlaySound("Teleport_Reappear");
				m_phase = Phase::Done;
			}
			return true;
		default:
			return false;
		}
	}

	void BossRoom::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		if (m_phase == Phase::Intro && m_time >= TITLE_AT)
		{
			// DisplayBossTitle: the two lines slide in from both sides, drift
			// 20 px, then leave after 3 s
			float t = (float) (m_time - TITLE_AT);
			float in = QuadOut(t), out = Clamp01((t - 3) / 0.6f);
			float a = Clamp01(t / 0.5f) * (1 - Clamp01((t - 3) / 0.5f));
			if (a > 0.01f)
			{
				uint32_t alpha = (uint32_t) (a * 255) << 24;
				float drift = 10 * Clamp01((t - 1) / 2);
				float x1 = 55 - 500 * (1 - in) + drift + 750 * out * out;
				float x2 = 55 + 750 * (1 - in) - drift - 500 * out * out;
				float y = 240 + 25;
				m_font.DrawOutlined(frame, m_title1.c_str(), x1, y, 0.36f, alpha | 0xFFFFFF, BLACK, 1);
				float w = 0, h = 0;
				m_font.Measure(m_title2.c_str(), w, h);
				float lineW = w * 0.8f * QuadOut(t);
				gfx_fill(55, (int) y + 22, (int) lineW, 2, a > 0.5f ? WHITE : 0xFF808080u);
				m_font.DrawOutlined(frame, m_title2.c_str(), x2, y + 25, 0.8f, alpha | 0xFFFFFF, BLACK, 2);
			}
		}
		// StartWipeTransition / EndWipeTransition: a short black flash
		if (m_fade > 0)
			gfx_fill(0, 0, SCREEN_W, SCREEN_H, 0xFF000000u);
	}
}
