#include "BossRoom.h"
#include "../../debugflags.h"
#include "../../SpriteSystem/SpriteLibrary.h"

#include "../../Audio/Audio.h"
#include "../../Text/Loc.h"
#include "../../platform.h"
#include "../GameObjects/BreakableObj.h"
#include "../GameObjects/ChestObj.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/EnemyObjs/EnemyObj_LastBoss.h"
#include "../GameObjects/Player/PlayerObj.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../Screens/EnemyNames.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "../Types/ChestType.h"
#include "../EVs/GlobalEV.h"
#include "../Types/ClassType.h"
#include "../Types/EnemyType.h"
#include "../Types/SpellType.h"
#include "../Types/TraitType.h"
#include "../Types/SpecialItemType.h"
#include "../Equipment/EquipmentData.h"
#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const float BORDER = 50;   // m_borderSize (100) on screen

		const char *EnemyLocID(const std::string &name)
		{
			for (const EnemyNameLoc &e : ENEMY_NAMES)
				if (name == e.name) return e.locID;
			return name.c_str();
		}

		float Clamp01(float t) { return t < 0 ? 0 : t > 1 ? 1 : t; }
		float QuadOut(float t) { t = Clamp01(t); return t * (2 - t); }
		float QuadIn(float t) { t = Clamp01(t); return t * t; }
		float QuadInOut(float t) { t = Clamp01(t); return t < 0.5f ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t); }
		float BounceOut(float t)
		{
			t = Clamp01(t);
			if (t < 1 / 2.75f) return 7.5625f * t * t;
			if (t < 2 / 2.75f) { t -= 1.5f / 2.75f; return 7.5625f * t * t + 0.75f; }
			if (t < 2.5f / 2.75f) { t -= 2.25f / 2.75f; return 7.5625f * t * t + 0.9375f; }
			t -= 2.625f / 2.75f;
			return 7.5625f * t * t + 0.984375f;
		}
		// Progress of a tween running from t0 to t1
		float Span(double t, double t0, double t1) { return Clamp01((float) ((t - t0) / (t1 - t0))); }

		// BlobBossRoom.DropBlobs: when each of the ten blobs starts to fall
		double BlobDrop(int i)
		{
			if (i < 2) return 0.5 + i;
			double delay = 1;
			for (int j = 2; j < i; j++) delay += 0.5 * (10 - j) / 10;
			return 1 + delay;
		}
		const double BLOB_TITLE = 4.8;   // DropBlobs' last delay + 1
	}

	BossRoom::BossRoom() = default;
	BossRoom::~BossRoom() { delete m_gate; }

	bool BossRoom::LoadContent(const char *assetsDir)
	{
		return m_font.Load(assetsDir, "Junicode");
	}

	void BossRoom::Reset()
	{
		m_phase = Phase::None;
		m_room = nullptr;
		m_bossChest.reset();
		m_fade = 0;
		m_gateClosed = false;
		m_gateTime = 0;
		m_gateX = m_gateY = 0;
		EndIntro();
		m_dialogue = nullptr;
	}

	void BossRoom::EndIntro()
	{
		m_orbs.clear();
		m_blobs.clear();
		m_shadow.reset();
		m_camKeys.clear();
		m_borders = 0;
		m_boss = nullptr;
	}

	// Each room's OnEnter: where the camera goes, what the boss does, when
	// the title comes and when the fight starts
	void BossRoom::StartIntro(ProceduralLevelScreen *level, Save::GameState &save)
	{
		PlayerObj *player = level->Player();
		DS2DEngine::Camera2D *cam = level->Camera();
		Rectangle rb = m_room->Bounds();
		if (m_room->LevelType() == Levels::LevelType::NONE)
			player->X(rb.Left() + 150.0f);   // LastBossRoom: in from the left door
		level->UpdateCamera();
		float playerX = cam->X(), playerY = cam->Y();
		m_camX0 = playerX;
		m_camY0 = playerY;
		m_camKeys.clear();
		if (m_boss != nullptr)
		{
			m_bossScale = m_boss->Scale().X;
			m_bossX = m_boss->X();
			m_bossY = m_boss->Y();
		}
		float bx = m_bossX, by = m_bossY;

		switch (m_room->LevelType())
		{
		case Levels::LevelType::GARDEN:
		{
			// FairyBossRoom: the camera sweeps the room while the boss's
			// black shadow flies past, then the boss fades in
			m_titleAt = 6.4;
			m_borders = 11;
			m_camKeys = { { 0, 1, rb.Left() + 900.0f, rb.Top() + 400.0f }, { 1.8, 2.8, rb.Right() - 900.0f, rb.Bottom() - 400.0f },
				{ 3.6, 4.6, bx, by }, { m_titleAt + 3.3, m_titleAt + 3.8, playerX, playerY } };
			m_introEnd = m_titleAt + 3.8;
			if (m_boss != nullptr) m_boss->Opacity(0);
			m_shadow.reset(new DS2DEngine::ObjContainer("EnemyFairyGhostBossMove_Character"));
			for (int i = 0; i < m_shadow->NumChildren(); i++) m_shadow->GetChildAt(i)->TextureColor(Color(0, 0, 0));
			m_shadow->Scale(Vector2(m_bossScale, m_bossScale));
			m_shadow->Position(Vector2(rb.Left() + 100.0f, rb.Top() + 400.0f));
			m_shadow->PlayAnimation(true);
			break;
		}
		case Levels::LevelType::TOWER:
		{
			// FireballBossRoom: fifteen fire orbs light up in a ring around
			// the boss, then it absorbs them one by one and grows
			m_titleAt = 5.5;
			m_borders = 0;
			m_camKeys = { { 0, 1, bx, by }, { m_titleAt + 3.5, m_titleAt + 4, playerX, playerY } };
			m_introEnd = m_titleAt + 4;
			for (int i = 0; i < 15; i++)
			{
				DS2DEngine::SpriteObj *orb = new DS2DEngine::SpriteObj("GhostBossProjectile_Sprite");
				float a = i * (360 / 15.0f) * 3.14159265f / 180;
				orb->Position(Vector2(bx + 300 * cosf(a), by + 300 * sinf(a)));
				orb->Scale(Vector2(2, 2));
				orb->Opacity(0);
				orb->PlayAnimation(true);
				m_orbs.emplace_back(orb);
			}
			if (m_boss != nullptr) m_boss->Scale(Vector2(0, 0));
			break;
		}
		case Levels::LevelType::DUNGEON:
		{
			// BlobBossRoom: ten small blobs fall from the ceiling into the
			// boss, which grows with each
			m_titleAt = BLOB_TITLE;
			m_borders = 9;
			m_camKeys = { { 0, 1, rb.Left() + 700.0f, by }, { m_titleAt + 3.5, m_titleAt + 4, playerX, playerY } };
			m_introEnd = m_titleAt + 4;
			for (int i = 0; i < 10; i++)
			{
				DS2DEngine::ObjContainer *blob = new DS2DEngine::ObjContainer("EnemyBlobBossAir_Character");
				blob->Position(Vector2(bx, by - 1000));
				blob->Scale(Vector2(0.4f, 0.4f));
				if (blob->NumChildren() > 2)
				{
					blob->GetChildAt(1)->TextureColor(Color(0, 0, 0));
					blob->GetChildAt(2)->TextureColor(Color(135, 206, 250));   // LightSkyBlue
					blob->GetChildAt(2)->Opacity(0.8f);
				}
				blob->PlayAnimation(true);
				m_blobs.emplace_back(blob);
			}
			if (m_boss != nullptr) m_boss->Scale(Vector2(0.5f, 0.5f));
			break;
		}
		case Levels::LevelType::NONE:
		{
			// LastBossRoom: the hero walks in, the camera moves on to the
			// Traitor, who talks before the title
			m_titleAt = 4.1;
			m_borders = 8;
			player->Flip(SpriteEffects::None);
			player->ChangeSprite("PlayerWalking_Character");
			player->PlayAnimation(true);
			float walkX = player->X() + 500 * 1.5f;   // MoveDirection at Speed for 1.5 s
			m_camKeys = { { 0, 1.5, walkX + GlobalEV::Camera_XOffset, playerY }, { 1.6, 3.1, walkX + GlobalEV::Camera_XOffset + 300, playerY },
				{ m_titleAt + 3, m_titleAt + 4, walkX + GlobalEV::Camera_XOffset, playerY } };
			m_introEnd = m_titleAt + 4;
			m_heroClass = save.hero.classType;
			break;
		}
		default:
		{
			// EyeballBossRoom: the camera rises to the eye, which spins five
			// times and swells from a dot
			m_titleAt = 3.2;
			m_borders = 8;
			m_camX0 = rb.Left() + cam->Bounds().Width * 0.5f;
			m_camY0 = player->Y();
			m_camKeys = { { 0, 1, m_camX0, by }, { m_titleAt + 3.5, m_titleAt + 4.5, m_camX0, rb.Bottom() - cam->Bounds().Height * 0.5f } };
			m_introEnd = m_titleAt + 4.5;
			if (m_boss != nullptr) m_boss->Scale(Vector2(0.3f, 0.3f));
			break;
		}
		}
		cam->X(m_camX0);
		cam->Y(m_camY0);
	}

	void BossRoom::UpdateIntro(ProceduralLevelScreen *level, double before)
	{
		const double t = m_time;
		auto at = [&](double when) { return before < when && t >= when; };
		PlayerObj *player = level->Player();

		// The camera: each key eases from where the last one ended
		float x = m_camX0, y = m_camY0;
		for (const CamKey &k : m_camKeys)
		{
			if (t <= k.t0) break;
			float e = QuadInOut(Span(t, k.t0, k.t1));
			x += (k.x - x) * e;
			y += (k.y - y) * e;
			if (t < k.t1) break;
		}
		level->Camera()->X(x);
		level->Camera()->Y(y);

		if (at(m_titleAt)) Audio::PlaySound("Boss_Title");
		if (at(m_titleAt + 3)) Audio::PlaySound("Boss_Title_Exit");

		switch (m_room->LevelType())
		{
		case Levels::LevelType::GARDEN:
			if (at(0.5)) Audio::PlaySound("Boss_Flameskull_Whoosh_01");
			if (at(2.0)) Audio::PlaySound("Boss_Flameskull_Whoosh_02");
			if (at(3.8)) Audio::PlaySound("Boss_Flameskull_Spawn");
			if (m_shadow != nullptr)
			{
				Rectangle rb = m_room->Bounds();
				m_shadow->X(rb.Left() + 100 + 3000 * Span(t, 1, 1.5) - 3000 * Span(t, 2.8, 3.3) + 1500 * QuadOut(Span(t, 4.6, 5.6)));
				m_shadow->Opacity(1 - Span(t, 5.9, 6.4));
			}
			if (m_boss != nullptr) m_boss->Opacity(Span(t, 5.9, 6.4));
			break;
		case Levels::LevelType::TOWER:
			for (int i = 0; i < (int) m_orbs.size(); i++)
			{
				DS2DEngine::SpriteObj *orb = m_orbs[i].get();
				double shown = 1.5 + 0.1 * i, absorbed = 3.5 + 0.1 * i;
				if (at(shown)) Audio::PlaySound("Boss_Fireball_Whoosh_01");
				if (at(absorbed))
				{
					Audio::PlaySound("Boss_Fireball_Puff_01");
					if (m_boss != nullptr) m_boss->Scale(m_boss->Scale() + Vector2(m_bossScale / 15, m_bossScale / 15));
				}
				float a = i * (360 / 15.0f) * 3.14159265f / 180, pull = QuadIn(Span(t, absorbed, absorbed + 0.2));
				orb->Position(Vector2(m_bossX + 300 * cosf(a) * (1 - pull), m_bossY + 300 * sinf(a) * (1 - pull)));
				orb->Opacity(QuadOut(Span(t, shown, shown + 0.2)) * (1 - Span(t, absorbed + 0.1, absorbed + 0.2)));
			}
			if (at(3.5)) Audio::PlaySound("Boss_Fireball_Spawn");
			break;
		case Levels::LevelType::DUNGEON:
			for (int i = 0; i < (int) m_blobs.size(); i++)
			{
				double drop = BlobDrop(i);
				m_blobs[i]->Y(m_bossY - 1000 + 1150 * QuadIn(Span(t, drop, drop + 1)));
				if (at(drop + 1))
				{
					// GrowBlob
					m_blobs[i]->Visible(false);
					if (m_boss != nullptr)
					{
						float grow = (m_bossScale - 0.5f) / 10;
						m_boss->Scale(m_boss->Scale() + Vector2(grow, grow));
					}
					Audio::PlaySound("Boss_Blob_Spawn_01", "Boss_Blob_Spawn_02", "Boss_Blob_Spawn_03");
				}
			}
			if (at(m_titleAt)) Audio::PlaySound("Boss_Blob_Spawn");
			break;
		case Levels::LevelType::NONE:
			if (t < 1.5)
				player->X(player->X() + 500 * (float) (t - before));
			if (at(1.5))
			{
				player->ChangeSprite("PlayerIdle_Character");
				player->PlayAnimation(true);
			}
			/* Dev/test: RL_DEBUG_BOSS_WIN skips the Traitor's talk: the
			 * dialogue waits for a button, which a headless test cannot
			 * press, and the intro would freeze on it. */
			if (at(3.1) && getenv("RL_DEBUG_BOSS_WIN") == nullptr)
				m_dialogue = m_heroClass == ClassType::Traitor ? "FinalBossTalk01_Special" : "FinalBossTalk01";
			break;
		default:
			if (at(0.8)) Audio::PlaySound("Boss_Eyeball_Build");
			if (m_boss != nullptr)
			{
				m_boss->Rotation(1800 * QuadInOut(Span(t, 1, 4)));
				float s = 0.3f + (m_bossScale - 0.3f) * BounceOut(Span(t, 2, 4));
				m_boss->Scale(Vector2(s, s));
			}
			break;
		}
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

	namespace
	{
		// The rematch heroes and their Neo bosses (the SetRoomData of
		// Eyeball/Fairy/Fireball/BlobChallengeRoom.cs): equipment and runes
		// by EquipmentCategoryType (Sword, Helm, Chest, Limbs, Cape), -1 none
		struct ChallengePreset
		{
			Levels::LevelType area;
			const char *name;
			bool female;
			uint8_t classType, spell, trait1, trait2;
			int specialItem, health, mana, strength, magic, defense;
			int8_t equipped[5], rune[5];
			int bossHealth[3], bossDamage[3];
		};
		namespace EB = EquipmentBaseType;
		namespace EA = EquipmentAbilityType;
		const ChallengePreset PRESETS[] = {
			{ Levels::LevelType::CASTLE, "McSwordy", true, ClassType::SpellSword2, SpellType::Close, TraitType::Vertigo, TraitType::Hyperactive,
			  SpecialItemType::Glasses, 39, 0, 5, 190, 0,
			  { EB::Silver, EB::Silver, EB::Silver, EB::Silver, EB::Silver }, { EA::MovementSpeed, EA::DoubleJump, EA::DoubleJump, EA::Dash, EA::Dash },
			  { 17000, 17000, 17000 }, { 57, 57, 57 } },
			{ Levels::LevelType::GARDEN, "Wagner", false, ClassType::Ninja2, SpellType::Translocator, TraitType::Tourettes, TraitType::OCD,
			  0, 30, 10, 150, 40, 230,
			  { EB::Dark, EB::Dark, EB::Spike, EB::Spike, EB::Sky }, { -1, -1, EA::DoubleJump, -1, EA::DoubleJump },
			  { 15000, 17000, 17000 }, { 200, 175, 175 } },
			{ Levels::LevelType::TOWER, "Dovahkiin", false, ClassType::Barbarian2, SpellType::None, TraitType::Dementia, TraitType::OCD,
			  0, 28, -14, 150, -10, 230,
			  { EB::Royal, EB::Royal, EB::Royal, EB::Royal, EB::Sky }, { EA::Dash, EA::DoubleJump, EA::DoubleJump, EA::DoubleJump, EA::DoubleJump },
			  { 12000, 12000, 12000 }, { 380, 380, 380 } },
			{ Levels::LevelType::DUNGEON, "Echidna", true, ClassType::Dragon, SpellType::DragonFireNeo, TraitType::Gay, TraitType::Hypogonadism,
			  0, 90, 8, 50, 33, 230,
			  { EB::Dragon, EB::Dragon, EB::Dragon, EB::Dragon, -1 }, { -1, EA::MovementSpeed, EA::MovementSpeed, -1, -1 },
			  { 100, 100, 100 }, { 370, 370, 370 } },
		};
		const ChallengePreset *PresetFor(Levels::LevelType area)
		{
			for (const ChallengePreset &p : PRESETS)
				if (p.area == area) return &p;
			return nullptr;
		}
		// RoomObj.m_empowerCostArray: the price of each upgrade
		const int EMPOWER_COST[] = { 7500, 15000, 30000, 45000, 67500, 90000, 120000, 150000, 187500, 225000 };
	}

	void BossRoom::StartChallenge(ProceduralLevelScreen *level, Save::GameState &save, RoomObj *room)
	{
		const ChallengePreset *p = PresetFor(room->LevelType());
		PlayerObj *player = level->Player();
		if (p == nullptr || player == nullptr) return;
		// StorePlayerData
		if (!m_challengeActive)
		{
			m_stored.hero = save.hero;
			memcpy(m_stored.equipped, save.equipment.equipped, 5);
			memcpy(m_stored.equippedRune, save.equipment.equippedRune, 5);
			m_stored.bonusHealth = save.BonusHealth;
			m_stored.bonusMana = save.BonusMana;
			m_stored.bonusStrength = save.BonusStrength;
			m_stored.bonusMagic = save.BonusMagic;
			m_stored.bonusDefense = save.BonusDefense;
			m_stored.specialItem = save.SpecialItem;
		}
		// SetRoomData: the preset hero, +20% health/strength/magic per upgrade
		int times = *ChallengeTimesFlag(save, (int) room->LevelType());
		if (times < 0) times = 0;
		save.hero.name = p->name;
		save.hero.romanNumeral = "";
		save.hero.isFemale = p->female;
		save.hero.classType = p->classType;
		save.hero.spell = p->spell;
		save.hero.trait1 = p->trait1;
		save.hero.trait2 = p->trait2;
		save.SpecialItem = p->specialItem;
		save.BonusHealth = p->health + (int) (p->health * 0.2f * times);
		save.BonusMana = p->mana;
		save.BonusStrength = p->strength + (int) (p->strength * 0.2f * times);
		save.BonusMagic = p->magic + (int) (p->magic * 0.2f * times);
		save.BonusDefense = p->defense;
		memcpy(save.equipment.equipped, p->equipped, 5);
		memcpy(save.equipment.equippedRune, p->rune, 5);
		m_challengeActive = true;
		m_challengeEntryX = player->X();
		player->ChallengeMode = true;
		player->ChallengeLost = false;
		player->SetPlayerStats(&save);
		player->CurrentHealth = player->MaxHealth();   // ChallengeBossRoomObj.OnEnter
		player->CurrentMana = player->MaxMana();
		// The Neo bosses
		int n = 0;
		for (EnemyObj *e : room->EnemyList())
			if (e != nullptr && !e->NonKillable)
			{
				int i = n < 3 ? n : 2;
				e->MaxHealth = (float) p->bossHealth[i];
				e->CurrentHealth = e->MaxHealth;
				e->Damage = (float) p->bossDamage[i];
				n++;
			}
	}

	void BossRoom::EndChallenge(ProceduralLevelScreen *level, Save::GameState &save)
	{
		// LoadPlayerData: the real hero comes back, healed
		if (!m_challengeActive) return;
		m_challengeActive = false;
		save.hero = m_stored.hero;
		memcpy(save.equipment.equipped, m_stored.equipped, 5);
		memcpy(save.equipment.equippedRune, m_stored.equippedRune, 5);
		save.BonusHealth = m_stored.bonusHealth;
		save.BonusMana = m_stored.bonusMana;
		save.BonusStrength = m_stored.bonusStrength;
		save.BonusMagic = m_stored.bonusMagic;
		save.BonusDefense = m_stored.bonusDefense;
		save.SpecialItem = m_stored.specialItem;
		if (PlayerObj *player = level->Player())
		{
			player->ChallengeMode = false;
			player->ChallengeLost = false;
			player->SetPlayerStats(&save);
			player->CurrentHealth = player->MaxHealth();
			player->CurrentMana = player->MaxMana();
		}
	}

	void BossRoom::Donate(Save::GameState &save)
	{
		int *times = ChallengeTimesFlag(save, m_donationArea);
		int cost = *times >= 0 && *times < 9 ? EMPOWER_COST[*times] : -1;
		if (cost < 0 || save.Gold < cost) return;
		save.Gold -= cost;
		(*times)++;
	}

	bool *BossRoom::ChallengeUnlockedFlag(Save::GameState &save, int levelType)
	{
		switch ((Levels::LevelType) levelType)
		{
		case Levels::LevelType::GARDEN: return &save.ChallengeSkullUnlocked;
		case Levels::LevelType::TOWER: return &save.ChallengeFireballUnlocked;
		case Levels::LevelType::DUNGEON: return &save.ChallengeBlobUnlocked;
		default: return &save.ChallengeEyeballUnlocked;
		}
	}

	bool *BossRoom::ChallengeBeatenFlag(Save::GameState &save, int levelType)
	{
		switch ((Levels::LevelType) levelType)
		{
		case Levels::LevelType::GARDEN: return &save.ChallengeSkullBeaten;
		case Levels::LevelType::TOWER: return &save.ChallengeFireballBeaten;
		case Levels::LevelType::DUNGEON: return &save.ChallengeBlobBeaten;
		default: return &save.ChallengeEyeballBeaten;
		}
	}

	int *BossRoom::ChallengeTimesFlag(Save::GameState &save, int levelType)
	{
		switch ((Levels::LevelType) levelType)
		{
		case Levels::LevelType::GARDEN: return &save.ChallengeSkullTimesUpgraded;
		case Levels::LevelType::TOWER: return &save.ChallengeFireballTimesUpgraded;
		case Levels::LevelType::DUNGEON: return &save.ChallengeBlobTimesUpgraded;
		default: return &save.ChallengeEyeballTimesUpgraded;
		}
	}

	/* The challenge door is reachable (OpenChallengeBossDoor): the boss is
	 * dead, the rematch paid for and not yet beaten. */
	bool BossRoom::ChallengeAccessible(RoomObj *room, Save::GameState &save)
	{
		return room->Name() == "EntranceBoss"
			&& room->ChallengeRoom() >= 0
			&& *BeatenFlag(save, (int) room->LevelType())
			&& *ChallengeUnlockedFlag(save, (int) room->LevelType())
			&& !*ChallengeBeatenFlag(save, (int) room->LevelType());
	}

	// The boss doors: an area's entrance until its boss is beaten; the
	// castle entrance once the four are (CastleEntranceRoomObj.OnEnter);
	// the corridor before the last boss (TutorialRoomObj)
	bool BossRoom::DoorOpen(RoomObj *room, Save::GameState &save)
	{
		if (room->Name() == "EntranceBoss")
			return !*BeatenFlag(save, (int) room->LevelType())
				|| ChallengeAccessible(room, save);
		if (room->Name() == "CastleEntrance")
			return save.EyeballBossBeaten && save.FairyBossBeaten && save.BlobBossBeaten && save.FireballBossBeaten;
		return room->Name() == "Tutorial";
	}

	const char *BossRoom::Music() const
	{
		if (m_room == nullptr || m_phase == Phase::Intro) return "";
		if (m_phase != Phase::Fight) return m_room->LevelType() == Levels::LevelType::NONE ? "" : "TitleScreenSong";   // the win stinger
		switch (m_room->LevelType())
		{
		case Levels::LevelType::NONE: return "LastBossSong";
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
		Rectangle b = player->TerrainBounds();   // the feet, not the sword
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

		/* Dev/test: RL_DEBUG_SHOOT fires an enemy shot at the hero every second */
		static bool debugShoot = getenv("RL_DEBUG_SHOOT") != nullptr;
		static double shootTimer = 2;
		if (debugShoot && (shootTimer -= dt) <= 0)
		{
			shootTimer = 1;
			EnemyObj *shooter = room->EnemyList().empty() ? nullptr : room->EnemyList()[0];
			level->FireProjectile("TurretProjectile_Sprite", player->Position() + Vector2(-200, -20), Vector2(1, 0),
			                      500, 10, Vector2(1, 1), false, shooter, 3, false);
		}
		/* Dev/test: RL_DEBUG_KILL=<s> kills the room's enemies after s seconds */
		static double debugKill = getenv("RL_DEBUG_KILL") != nullptr ? atof(getenv("RL_DEBUG_KILL")) : -1;
		if (debugKill > 0 && (debugKill -= dt) <= 0)
			level->DamageAllEnemies(1000000);

		/* Dev/test: RL_DEBUG_BOSS=<LevelType 1-4> puts the hero at that
		 * area's boss door, 5 at the last boss's; with an "i" suffix
		 * ("3i") he teleports straight INTO the boss room, so the intro
		 * and the fight start without walking in (headless tests). */
		static const char *debugBoss = getenv("RL_DEBUG_BOSS");
		if (debugBoss != nullptr)
		{
			char *end = nullptr;
			long type = strtol(debugBoss, &end, 10);
			bool straightIn = end != nullptr && *end == 'i';
			debugBoss = nullptr;
			for (RoomObj *r : level->RoomList())
				if (type == 5 ? r->Name() == "Tutorial" : r->Name() == "EntranceBoss" && (int) r->LevelType() == type)
				{
					RoomObj *boss = straightIn && r->LinkedRoom() >= 0
					                && r->LinkedRoom() < (int) level->RoomList().size()
					                    ? level->RoomList()[r->LinkedRoom()] : nullptr;
					/* straight in: at the boss room's own door, just
					 * inside - the room transition only triggers when
					 * the hero crosses a door, and the fight only
					 * starts once the level knows he is in the room. */
					Teleport(level, boss != nullptr ? boss : r, true);
					fprintf(stderr, "boss: %s area %ld, boss room %d\n",
					        boss != nullptr ? "inside the door of" : "at the door of",
					        type, r->LinkedRoom());
					return true;
				}
		}

		/* Dev/test: RL_DEBUG_NEAR=<EnemyType>[,dx] puts the hero dx (default
		 * -150) beside the first enemy of that type outside the boss rooms;
		 * RL_DEBUG_NEAR=b<n> beside the n-th breakable. */
		static const char *debugNear = getenv("RL_DEBUG_NEAR");
		if (debugNear != nullptr)
		{
			const char *arg = debugNear;
			debugNear = nullptr;
			bool breakable = arg[0] == 'b', hazard = arg[0] == 'h';   // h<n>: beside the n-th spikes
			bool slope = arg[0] == 's';                                 // s<n>: beside the n-th ramp's top
			bool chest = arg[0] == 'c';                                 // c<n>: beside the n-th chest
			int want = atoi(breakable || hazard || slope || chest ? arg + 1 : arg), seen = 0;
			const char *comma = strchr(arg, ',');
			float dx = comma != nullptr ? (float) atof(comma + 1) : -150;
			const char *comma2 = comma != nullptr ? strchr(comma + 1, ',') : nullptr;
			float dy = comma2 != nullptr ? (float) atof(comma2 + 1) : -60;   // [,dy]
			for (RoomObj *r : level->RoomList())
			{
				if (r->Name() == "Boss") continue;
				DS2DEngine::GameObj *target = nullptr;
				if (slope)
				{
					for (const Slope &s : r->SlopeList())
						if (seen++ == want)
						{
							player->X(s.x + dx);
							player->Y(s.y + dy);
							fprintf(stderr, "near: ramp top %.0f,%.0f w%.0f h%.0f in %s\n", s.x, s.y, s.width, s.height, r->Name().c_str());
							return false;
						}
					continue;
				}
				if (hazard)
				{
					for (const Hazard &h : r->HazardList())
						if (seen++ == want)
						{
							player->X(h.x + dx);
							player->Y(h.y + dy);
							fprintf(stderr, "near: spikes %.0f,%.0f %.0fx%.0f in %s\n", h.x, h.y, h.width, h.height, r->Name().c_str());
							return false;
						}
					continue;
				}
				if (chest)
				{
					for (ChestObj *ch : r->ChestList())
						if (seen++ == want) { target = ch; fprintf(stderr, "near: chest type %d locked %d open %d in %s\n", (int) ch->ChestType(), (int) ch->IsLocked, (int) ch->IsOpen(), r->Name().c_str()); break; }
				}
				else if (breakable)
				{
					for (BreakableObj *b : r->BreakableObjList())
						if (seen++ == want) { target = b; break; }
				}
				else
					for (EnemyObj *e : r->EnemyList())
						if (e != nullptr && e->Type == want) { target = e; break; }
				if (target == nullptr) continue;
				player->X(target->X() + dx);
				player->Y(target->Y() + dy);
				fprintf(stderr, "near: %s at %.0f,%.0f in %s\n", target->SpriteName().c_str(), target->X(), target->Y(), r->Name().c_str());
				return false;
			}
		}

		// The boss door: Up in front of it, while that boss lives - or
		// to the rematch once it is paid for (OpenChallengeBossDoor)
		if ((pressed & BTN_UP) && player->IsTouchingGround() && DoorOpen(room, save)
			&& room->LinkedRoom() >= 0 && room->LinkedRoom() < (int) level->RoomList().size())
		{
			/* the challenge room when the rematch is accessible */
			int to = room->LinkedRoom();
			if (ChallengeAccessible(room, save)
				&& room->ChallengeRoom() < (int) level->RoomList().size())
				to = room->ChallengeRoom();
			Rectangle b = player->Bounds();
			for (const RoomObj::DoorInfo &d : room->DoorList())
			{
				float dx = d.x, dy = d.y;
				if (d.bossDoor && b.Right() > dx && b.X < dx + d.width && b.Bottom() > dy && b.Top() < dy + d.height)
				{
					Teleport(level, level->RoomList()[to], true);
					return true;
				}
			}
		}

		if (room->Name() == "CastleEntrance")
		{
			save.LoadStartingRoom = false;   /* CastleEntranceRoomObj.cs line 169 */
			UpdateGate(level, room, dt);
		}

		/* KickPlayerOut: the preset hero lost the rematch; the real one is
		 * back at the boss door, alive */
		if (m_challengeActive && player->ChallengeLost && m_room != nullptr)
		{
			int back = m_room->LinkedRoom();
			EndChallenge(level, save);
			if (back >= 0 && back < (int) level->RoomList().size())
				Teleport(level, level->RoomList()[back], true);
			Audio::PlaySound("Teleport_Reappear");
			m_phase = Phase::None;
			m_room = nullptr;
			return true;
		}

		/* The donation box in front of a reachable rematch (RoomObj.OnEnter
		 * shows it, Up talks to it) */
		if (room->Name() == "EntranceBoss" && (pressed & BTN_UP) && player->IsTouchingGround()
		    && ChallengeAccessible(room, save))
		{
			const std::vector<RoomObj *> &rooms = level->RoomList();
			for (size_t i = 0; i < rooms.size() && i < level->Castle().rooms.size(); i++)
			{
				if (rooms[i] != room) continue;
				for (const Levels::ObjData &o : level->Castle().rooms[i].objs)
				{
					if (o.name != "donationbox") continue;
					Rectangle box((int) o.x, (int) o.y, (int) (o.width * o.scaleX), (int) (o.height * o.scaleY));
					if (!box.Intersects(player->Bounds())) continue;
					int times = *ChallengeTimesFlag(save, (int) room->LevelType());
					m_donationArea = (int) room->LevelType();
					m_donationCost = times >= 0 && times < 9 ? EMPOWER_COST[times] : -1;
					m_donationOffer = true;
				}
			}
		}

		if (rl_trace())
		{
			static int lastPhase = -1;
			if ((int) m_phase != lastPhase)
				fprintf(stderr, "boss phase %d in %s (challenge %d)\n", (int) m_phase, room->Name().c_str(), (int) m_challengeActive);
			lastPhase = (int) m_phase;
		}
		bool challenge = room->Name() == "ChallengeBoss";
		if (room->Name() != "Boss" && !challenge)
		{
			if (m_room != nullptr && room != m_room) m_room = nullptr, m_phase = Phase::None;
			return false;
		}

		if (room != m_room)
		{
			// BossRoomObj.OnEnter / ChallengeBossRoomObj.OnEnter
			m_room = room;
			m_time = 0;
			if (challenge)
			{
				/* the rematch: done once beaten; otherwise the preset hero
				 * (empowered by the donation box) against the Neo boss */
				m_phase = *ChallengeBeatenFlag(save, (int) room->LevelType()) ? Phase::Done : Phase::Intro;
				if (m_phase == Phase::Intro)
					StartChallenge(level, save, room);
			}
			else
				m_phase = room->LevelType() != Levels::LevelType::NONE && *BeatenFlag(save, (int) room->LevelType())
					? Phase::Done : Phase::Intro;
			if (m_phase == Phase::Intro)
			{
				Audio::StopMusic();
				const char *title = "LOC_ID_ENEMY_NAME_118";   // the Lookout (Khidr)
				switch (room->LevelType())
				{
				case Levels::LevelType::GARDEN: title = "LOC_ID_ENEMY_NAME_120"; break;
				case Levels::LevelType::TOWER: title = "LOC_ID_ENEMY_NAME_122"; break;
				case Levels::LevelType::DUNGEON: title = "LOC_ID_ENEMY_NAME_119"; break;
				case Levels::LevelType::NONE: title = "LOC_ID_ENEMY_NAME_123"; break;   // LastBossRoom
				default: break;
				}
				m_title1 = Text::Loc::Get(title);
				m_title2.clear();
				m_boss = nullptr;
				for (EnemyObj *e : room->EnemyList())
					if (e != nullptr && !e->NonKillable && e->Type != EnemyType::SpikeTrap) { m_boss = e; m_title2 = Text::Loc::Get(EnemyLocID(e->Name)); break; }
				StartIntro(level, save);
			}
		}
		m_time += dt;

		switch (m_phase)
		{
		case Phase::Intro:
		{
			UpdateIntro(level, m_time - dt);
			if (m_time >= m_introEnd)
			{
				// EndCutscene: the boss as it was, the camera back on the hero
				if (m_boss != nullptr)
				{
					m_boss->Scale(Vector2(m_bossScale, m_bossScale));
					m_boss->Rotation(0);
					m_boss->Opacity(1);
				}
				EndIntro();
				level->UpdateCamera();
				m_phase = Phase::Fight;
			}
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
			if (any && !alive && room->LevelType() == Levels::LevelType::NONE)
			{
				// LastBossRoom.BossKilled: the second form is dead
				bool second = false;
				for (EnemyObj *e : room->EnemyList())
					if (e != nullptr && e->Type == EnemyType::LastBoss)
						second = static_cast<EnemyObj_LastBoss *>(e)->IsSecondForm();
				if (!second) return false;
				Audio::PlaySound("FinalBoss_St2_WeatherChange_b");
				save.NewBossBeaten = true;
				m_phase = Phase::Ending;
				m_time = 0;
				return false;
			}
			if (any && !alive)
			{
				// BossCleanup: the flag is saved, the boss chest falls
				if (m_room->Name() == "ChallengeBoss")
					*ChallengeBeatenFlag(save, (int) room->LevelType()) = true;
				else
					*BeatenFlag(save, (int) room->LevelType()) = true;
				save.NewBossBeaten = true;
				m_phase = Phase::Won;
				m_bossChest.reset();
				if (rl_trace()) fprintf(stderr, "boss: beaten\n");
				m_time = 0;
			}
			return false;
		}
		case Phase::Won:
		{
			if (m_bossChest == nullptr)
			{
				// ChestType.Boss: three stat drops
				m_bossChest.reset(new ChestObj());
				m_bossChest->SetChestType(ChestType::Boss);
				m_bossChest->X(m_challengeActive ? m_challengeEntryX : room->X() + room->Width() / 2.0f);
				// m_roomFloor: the lowest terrain top under the chest
				float cx = m_bossChest->X(), floor = room->Y() + room->Height() - 120;
				int best = INT32_MIN;
				for (TerrainObj *t : room->TerrainObjList())
				{
					Rectangle b = t->TerrainBounds();
					if (cx >= b.Left() && cx <= b.Right() && b.Top() > best && b.Top() < room->Y() + room->Height())
						best = b.Top();
				}
				m_bossChest->Y(0);
				if (best != INT32_MIN) floor = best - (m_bossChest->Bounds().Bottom() - 0);
				m_chestFloor = floor;
			}
			float t = (float) (m_time / 4 > 1 ? 1 : m_time / 4);
			m_bossChest->Opacity(t);
			m_bossChest->Y(m_chestFloor - 600 * (1 - t) * (1 - t));   // Quad.EaseOut
			m_bossChest->ArrowVisible = false;
			if (rl_trace() && t >= 1 && player != nullptr)
			{
				static int n = 0;
				if (n++ % 60 == 0)
					fprintf(stderr, "chest %.0f,%.0f hero %.0f,%.0f touch %d ground %d locked %d\n", m_bossChest->X(), m_bossChest->Y(), player->X(), player->Y(),
					        (int) m_bossChest->Bounds().Intersects(player->Bounds()), (int) player->IsTouchingGround(), (int) player->ControlsLocked);
			}
			if (t >= 1 && player != nullptr && m_bossChest->Bounds().Intersects(player->Bounds())
			    && player->IsTouchingGround() && !player->ControlsLocked)
			{
				m_bossChest->ArrowVisible = true;
				if (pressed & BTN_UP)
				{
					m_bossChest->ArrowVisible = false;
					m_bossChest->OpenChest(level, player);
					m_phase = Phase::Chest;
					m_time = 0;
				}
			}
			return false;
		}
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
				EndChallenge(level, save);   // LoadPlayerData
				int back = room->LinkedRoom();
				if (back >= 0 && back < (int) level->RoomList().size())
					Teleport(level, level->RoomList()[back], true);
				Audio::PlaySound("Teleport_Reappear");
				m_phase = Phase::Done;
			}
			return true;
		case Phase::Ending:
			// The FinalBossDoor opens; the ending follows
			if (m_time >= 4) m_phase = Phase::Finished;
			return false;
		default:
			return false;
		}
	}

	void BossRoom::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		// RunCinematicBorders: two black bands, gone in the last second
		if (m_phase == Phase::Intro && m_borders > 0)
		{
			int h = (int) (BORDER * (1 - QuadInOut(Span(m_time, m_borders - 1, m_borders))));
			if (h > 0)
			{
				gfx_fill(0, 0, SCREEN_W, h, BLACK);
				gfx_fill(0, SCREEN_H - h, SCREEN_W, h, BLACK);
			}
		}
		if (m_phase == Phase::Intro && m_time >= m_titleAt)
		{
			// DisplayBossTitle: the two lines slide in from both sides, drift
			// 20 px, then leave after 3 s
			float t = (float) (m_time - m_titleAt);
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

	void BossRoom::DrawWorld(DS2DEngine::Camera2D *camera)
	{
		if (m_gate != nullptr && m_gateX != 0)
		{
			m_gate->Position(Vector2(m_gateX, m_gateY + m_gate->Height() * QuadOut((float) (m_gateTime / 1.5))));
			m_gate->Draw(camera);
		}
		if (m_bossChest != nullptr && (m_phase == Phase::Won || m_phase == Phase::Chest || m_phase == Phase::Leaving))
			m_bossChest->Draw(camera);
		if (m_phase != Phase::Intro) return;
		if (m_shadow != nullptr) m_shadow->Draw(camera);
		for (auto &orb : m_orbs) if (orb->Opacity() > 0) orb->Draw(camera);
		for (auto &blob : m_blobs) if (blob->Visible()) blob->Draw(camera);
	}

	void BossRoom::UpdateGate(ProceduralLevelScreen *level, RoomObj *room, double dt)
	{
		if (m_gate == nullptr)
		{
			if (!SpriteSystem::SpriteLibrary::ContainsSprite("CastleEntranceGate_Sprite")) return;
			m_gate = new DS2DEngine::SpriteObj("CastleEntranceGate_Sprite");
			m_gate->ForceDraw(true);
			m_gate->Anchor(Vector2(0, 0));
		}
		if (m_gateX == 0)
		{
			// above the left door, flush with its right edge (line 108)
			for (const RoomObj::DoorInfo &d : room->DoorList())
				if (d.pos == Levels::DoorPos::Left)
				{
					m_gateX = d.x + d.width - m_gate->Width();
					m_gateY = d.y - m_gate->Height();
					break;
				}
			if (m_gateX == 0) return;
		}
		PlayerObj *player = level->Player();
		if (!m_gateClosed)
		{
			// CloseGate(true): it drops over 1.5 s (Quad.EaseOut)
			m_gateClosed = true;
			m_gateTime = 0;
		}
		if (m_gateTime < 1.5) m_gateTime += dt;
		// Update: no going back to the courtyard
		float right = m_gateX + m_gate->Width();
		if (player != nullptr && player->X() < right)
		{
			player->X(right + 20);
			level->UpdateCamera();
		}
	}
}
