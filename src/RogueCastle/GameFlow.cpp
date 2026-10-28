#include "GameFlow.h"

#include "../Audio/Audio.h"
#include "../Text/Loc.h"
#include "../Text/LocIDs.h"
#include "../platform.h"
#include "Game.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace RogueCastle
{
	bool GameFlow::Init(const char *assetsDir, const char *savePath)
	{
		m_assets = assetsDir;
		m_savePath = savePath;
		Text::Loc::Load(assetsDir, "fr");
		Audio::Init(assetsDir);
		m_options.LoadContent(assetsDir, "config.txt");
		m_generator = Lineage::Generator((uint32_t) time(nullptr));
		std::string names = m_assets + "/HeroNames.txt", female = m_assets + "/HeroineNames.txt";
		m_generator.LoadNames(names.c_str(), female.c_str());
		bool ok = m_lineage.LoadContent(assetsDir) && m_manor.LoadContent(assetsDir) && m_hud.LoadContent(assetsDir)
			&& m_map.LoadContent(assetsDir) && m_pause.LoadContent(assetsDir)
			&& m_court.LoadContent(assetsDir) && m_dialogue.LoadContent(assetsDir) && m_smith.LoadContent(assetsDir) && m_enchant.LoadContent(assetsDir);
		Save::Read(m_save, savePath);
		if (getenv("RL_DEBUG_LEVEL") == nullptr)
		{
			OpenTitle();
			return ok;
		}
		LeaveTitle();
		return ok;
	}

	void GameFlow::OpenTitle()
	{
		// InitializeStartingText: continue / choose an heir / new legacy
		const char *text = m_save.hero.name.empty() ? "LOC_ID_TITLE_SCREEN_4"
			: m_save.IsDead ? "LOC_ID_TITLE_SCREEN_3" : "LOC_ID_TITLE_SCREEN_2";
		m_title.Open(m_assets.c_str(), text);
		m_state = State::Title;
	}

	void GameFlow::LeaveTitle()
	{
		m_title.Close();
		// A saved hero still alive goes straight back to the castle
		if (!m_save.hero.name.empty() && !m_save.IsDead) StartCastle();
		else if (getenv("RL_DEBUG_LEVEL") != nullptr)
		{
			// Dev/test: skip the heir choice and jump into a fresh castle.
			std::vector<Lineage::Heir> heirs =
			    m_generator.CreateBranches(m_save.skills.ClassUnlocks(false), m_save.FamilyTreeNames());
			m_save.hero = heirs.empty() ? Lineage::Heir() : heirs.front();
			StartCastle();
		}
		else
		{
			m_state = State::Lineage;
			m_lineage.Open(m_save, m_generator);
		}
	}

	void GameFlow::Save()
	{
		/* The hero's vitals persist between runs (PlayerStats). */
		if (m_level != nullptr && m_level->Player() != nullptr && m_state == State::Castle)
			m_save.CurrentHealth = (int) m_level->Player()->CurrentHealth;
		Save::Write(m_save, m_savePath.c_str());
	}

	void GameFlow::StartCastle()
	{
		delete m_level;
		m_court.ResetToll(m_save.TimesDead > 0);   // TollCollectorAvailable
		if (!m_save.castleExists)
		{
			m_save.castleExists = true;
			m_save.castleSeed = (uint32_t) time(nullptr);
			m_save.roomsVisited.clear();
			m_save.chestsOpened.clear();
		}
		m_save.IsDead = false;
		m_level = new ProceduralLevelScreen(m_assets.c_str(), m_save.castleSeed);
		/* The hero's real stats (Game.PlayerStats) drive the player. */
		m_level->Player()->SetPlayerStats(&m_save);
		if (getenv("RL_DEBUG_TRACE") != nullptr)
			fprintf(stderr, "hero-stats: hp=%.0f/%.0f mp=%.0f dmg=%d magic=%d armor=%.0f weight=%d\n",
			        m_level->Player()->MaxHealth(), m_level->Player()->MaxHealth(),
			        m_level->Player()->MaxMana(), m_level->Player()->TotalDamage(),
			        m_level->Player()->TotalMagicDamage(), m_level->Player()->TotalArmor(),
			        m_level->Player()->MaxWeight());
		float maxHealth = m_level->Player()->MaxHealth();
		m_level->Player()->CurrentHealth =
			m_save.CurrentHealth > 0 && m_save.CurrentHealth <= maxHealth
			? (float) m_save.CurrentHealth : maxHealth;
		m_state = State::Castle;
		m_mapOpen = false;
		m_paused = false;
		m_map.Reset(m_level->Castle().rooms.size());
		Save();
	}

	// ProceduralLevelScreen death -> GameOverScreen -> SkillScreen
	void GameFlow::OnPlayerDeath()
	{
		Save::FamilyTreeNode node;
		const Lineage::Heir &h = m_save.hero;
		node.name = h.name;
		node.romanNumeral = h.romanNumeral;
		node.age = h.age;
		node.childAge = h.childAge;
		node.classType = h.classType;
		node.headPiece = h.headPiece;
		node.chestPiece = h.chestPiece;
		node.shoulderPiece = h.shoulderPiece;
		node.isFemale = h.isFemale;
		node.trait1 = h.trait1;
		node.trait2 = h.trait2;
		node.numEnemiesBeaten = m_save.NumEnemiesBeaten;
		node.beatenABoss = m_save.NewBossBeaten;
		m_save.familyTree.push_back(node);
		m_save.IsDead = true;
		m_save.TimesDead++;
		m_save.NumEnemiesBeaten = 0;
		m_save.NewBossBeaten = false;
		/* A new castle for the next heir, unless the architect locked this
		 * one (LoadingScreen: the lock is used up by the next load) */
		m_save.castleExists = m_save.LockCastle;
		m_save.LockCastle = false;
		m_save.HasArchitectFee = false;
		Save();
		delete m_level;
		m_level = nullptr;
		m_deathTimer = 0;
		std::string name = Text::Loc::Format(h.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
			h.name, h.romanNumeral);
		m_gameOver.Open(m_assets.c_str(), m_lastFrame, name, h.isFemale);
		m_lastFrame.clear();
		m_state = State::GameOver;
	}

	void GameFlow::Update(uint32_t buttons, double dt)
	{
		uint32_t pressed = buttons & ~m_previous;
		m_previous = buttons;
		UpdateMusic();
		if (m_optionsOpen)
		{
			switch (m_options.Update(pressed, dt))
			{
			case OptionsScreen::Result::Open: return;
			case OptionsScreen::Result::Closed: m_optionsOpen = false; return;
			case OptionsScreen::Result::Quit: Save(); m_quit = true; return;
			case OptionsScreen::Result::BackToTitle:
				m_optionsOpen = false;
				m_paused = m_mapOpen = false;
				Save();
				delete m_level;
				m_level = nullptr;
				OpenTitle();
				return;
			}
		}
		switch (m_state)
		{
		case State::Lineage:
			/* The new heir starts in the courtyard (the castle's first room) */
			if (m_lineage.Update(buttons, pressed, dt)) StartCastle();
			break;
		case State::Title:
			if (pressed & BTN_SELECT)
			{
				m_options.Open(false);
				m_optionsOpen = true;
				break;
			}
			if (m_title.Update(pressed, dt))
			{
				Audio::PlaySound("Game_Start");
				LeaveTitle();
			}
			break;
		case State::Shop:
			if (m_shop == nullptr || m_shop->Update(buttons, pressed, dt))
			{
				Save();
				m_state = State::Castle;
			}
			break;
		case State::GameOver:
			if (m_gameOver.Update(pressed, dt))
			{
				m_gameOver.Close();
				m_state = State::Lineage;
				m_lineage.Open(m_save, m_generator);
			}
			break;
		case State::Manor:
			if (m_manor.Update(buttons, pressed, dt))
			{
				Save();
				/* Back to the courtyard, walking in from the left */
				if (m_level != nullptr && m_level->Player() != nullptr)
				{
					StartingRoom::PlaceAtLeftEdge(m_level->Player());
					m_state = State::Castle;
				}
				else StartCastle();
			}
			break;
		case State::Castle:
		{
			if (m_dialogue.IsOpen())
			{
				DialogueScreen::Result r = m_dialogue.Update(pressed, dt);
				if (m_tollAsking && r != DialogueScreen::Result::Open)
				{
					m_tollAsking = false;
					if (r == DialogueScreen::Result::Confirmed)
					{
						/* TollPaid: GATEKEEPER_TOLL_COST (all the gold) minus Haggle */
						float cost = 1.0f - m_save.skills.ModifierAmount(Skills::SkillType::Prices_Down);
						m_save.Gold -= (int) (m_save.Gold * cost);
						m_save.SpokeToTollCollector = true;
						m_court.HideToll();
						Audio::PlaySound("Charon_LaughPoof");
						Save();
					}
				}
				if (m_architectAsking && r != DialogueScreen::Result::Open)
				{
					m_architectAsking = false;
					if (r == DialogueScreen::Result::Confirmed)
					{
						/* ActivateArchitect / StopArchitectActivation: this castle
						 * stays for the next heir; the gold fee applies until then */
						m_save.LockCastle = true;
						m_save.HasArchitectFee = true;
						m_save.ArchitectUsed = true;
						Save();
						Audio::PlaySound("Architect_Lever");
						m_dialogue.Open("Castle Lock Complete Architect", m_save.hero.isFemale);
					}
				}
				break;
			}
			if (m_paused)
			{
				if (pressed & BTN_SELECT)
				{
					m_options.Open(true);
					m_optionsOpen = true;
					break;
				}
				m_paused = m_pause.Update(pressed);
				if (!m_paused) Audio::PlaySound("Resume_Toggle");
				break;
			}
			if (m_mapOpen)
			{
				m_mapOpen = m_map.Update(buttons, pressed, dt);
				if (!m_mapOpen) Audio::PlaySound("Map_Off");
				break;
			}
			PlayerObj *player = m_level->Player();
			if (player != nullptr && !player->IsKilled && (pressed & BTN_START) && !(buttons & BTN_SELECT))
			{
				m_paused = true;
				Audio::PlaySound("Pause_Toggle");
				break;
			}
			if (player != nullptr && (pressed & BTN_SELECT) && !(buttons & BTN_START))
			{
				m_map.Open(player->X(), player->Y());
				m_mapOpen = true;
				Audio::PlaySound("Map_On");
				break;
			}
			if (player != nullptr && InCourtyard())
			{
				StartingRoom::Action action = m_court.Update(player, m_save.skills, pressed, dt);
				if (action == StartingRoom::Action::Blacksmith || action == StartingRoom::Action::Enchantress)
				{
					m_shop = action == StartingRoom::Action::Blacksmith ? &m_smith : &m_enchant;
					m_shop->Open(m_save, MaxWeight());
					Audio::PlaySound("ShopMenuOpen");
					m_state = State::Shop;
					break;
				}
				if (action == StartingRoom::Action::Toll)
				{
					// "Meet Toll Collector 1", then "...Skip<Haggle %>"
					char name[48];
					if (!m_save.SpokeToTollCollector) snprintf(name, sizeof(name), "Meet Toll Collector 1");
					else snprintf(name, sizeof(name), "Meet Toll Collector Skip%d",
						(int) (m_save.skills.ModifierAmount(Skills::SkillType::Prices_Down) * 100 + 0.5f));
					m_dialogue.Open(name, m_save.hero.isFemale, "ConfirmTest1");
					m_tollAsking = true;
					break;
				}
				if (action == StartingRoom::Action::Architect)
				{
					// StartingRoomObj: the lock offer, or "already locked"
					if (m_save.LockCastle)
						m_dialogue.Open("Castle Already Locked Architect", m_save.hero.isFemale);
					else
					{
						m_dialogue.Open(m_save.SpokeToArchitect ? "Meet Architect 2" : "Meet Architect", m_save.hero.isFemale, "ConfirmTest1");
						m_save.SpokeToArchitect = true;
						m_architectAsking = true;
					}
					break;
				}
				if (action == StartingRoom::Action::Manor)
				{
					Save();
					m_state = State::Manor;
					m_manor.Open(m_save);
					break;
				}
			}
			GameTime gameTime(dt);
			Game::GlobalInput.Update(buttons);
			m_level->Update(&gameTime);
			/* The real death: PlayerObj hides the hero when the health
			 * runs out; give it a short pause before the lineage flow
			 * (the original waits for the death animation). */
			/* Dev/test: RL_DEBUG_DIE=<seconds> kills the hero after that long */
			static const char *debugDie = getenv("RL_DEBUG_DIE");
			static double alive = 0;
			alive += dt;
			bool dying = debugDie != nullptr && alive >= atof(debugDie);
			if (m_level->Player() != nullptr && (m_level->Player()->IsKilled || dying))
			{
				m_deathTimer += dt;
				if (m_deathTimer >= 1.0)
				{
					m_deathTimer = 0;
					OnPlayerDeath();
				}
			}
			break;
		}
		}
	}

	void GameFlow::Draw(uint32_t *frame, double drawElapsed)
	{
		DrawState(frame, drawElapsed);
		if (m_optionsOpen) m_options.Draw(frame);
	}

	void GameFlow::DrawState(uint32_t *frame, double drawElapsed)
	{
		switch (m_state)
		{
		case State::Title:
			m_title.Draw(frame);
			break;
		case State::Lineage:
			m_lineage.Draw(frame);
			break;
		case State::Shop:
			if (m_shop != nullptr) m_shop->Draw(frame);
			break;
		case State::GameOver:
			m_gameOver.Draw(frame);
			break;
		case State::Manor:
			m_manor.Draw(frame);
			break;
		case State::Castle:
		{
			gfx_begin(frame);
			gfx_clear(0xFF000000u);
			m_level->Camera()->ElapsedTotalSeconds(drawElapsed);
			m_level->Draw(m_level->Camera());
			if (InCourtyard()) m_court.Draw(m_level->Camera(), m_save.skills);
			HUDValues v;
			/* The hero's real vitals (PlayerObj health, the save mana:
			 * spells are not cast yet, so mana only comes from stats). */
			if (m_level->Player() != nullptr)
			{
				v.maxHealth = (int) m_level->Player()->MaxHealth();
				v.health = (int) m_level->Player()->CurrentHealth;
			}
			v.maxMana = 100;
			v.mana = m_save.CurrentMana;
			v.gold = m_save.Gold;
			v.level = m_save.skills.PlayerLevel();
			v.heroName = Text::Loc::Format(m_save.hero.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
				m_save.hero.name, m_save.hero.romanNumeral);
			m_hud.Draw(frame, v);
			if (m_level->Player() != nullptr)
				m_map.Visit(m_level->Castle(), m_level->Player()->X(), m_level->Player()->Y());
			if (m_deathTimer > 0) m_lastFrame.assign(frame, frame + (size_t) SCREEN_W * SCREEN_H);
			if (InCourtyard()) m_court.DrawRain(frame, m_level->Camera());
			/* The minimap (not in the courtyard nor the boss rooms) */
			if (m_level->Player() != nullptr && !InCourtyard() && !m_mapOpen)
			{
				const Levels::PlacedRoom *room = nullptr;
				float px = m_level->Player()->X(), py = m_level->Player()->Y();
				for (const Levels::PlacedRoom &r : m_level->Castle().rooms)
					if (px >= r.x && px < r.Right() && py >= r.y && py < r.Bottom()) room = &r;
				if (room == nullptr || (room->name != "Boss" && room->name != "ChallengeBoss"))
					m_map.DrawMini(frame, m_level->Castle(), px, py, 1 / 60.0);
			}
			if (m_dialogue.IsOpen()) m_dialogue.Draw(frame);
			if (m_mapOpen) m_map.Draw(frame, m_level->Castle());
			else if (m_paused)
				m_pause.Draw(frame, m_level->Camera(), m_level->Player(), m_level->CurrentRoom(), v.heroName,
					Text::Loc::Get(LocIDs::ClassType_ToStringID(m_save.hero.classType, m_save.hero.isFemale)));
			break;
		}
		}
	}
}

namespace RogueCastle
{
	// The hero stands in the castle's first room, the courtyard
	bool GameFlow::InCourtyard() const
	{
		if (m_level == nullptr || m_level->Player() == nullptr || m_level->Castle().rooms.empty()) return false;
		const Levels::PlacedRoom &r = m_level->Castle().rooms[0];
		float x = m_level->Player()->X(), y = m_level->Player()->Y();
		return r.name == "Start" && x < r.Right() && y >= r.y && y < r.Bottom();
	}

	// PlayerObj.MaxWeight: BaseWeight 50 + Equip_Up skills + weight drops * 5
	int GameFlow::MaxWeight() const
	{
		return Equipment::EquipmentSystem::MaxWeight(50, m_save.skills.ModifierAmount(Skills::SkillType::Equip_Up),
			m_save.skills.ModifierAmount(Skills::SkillType::Equip_Up_Final), m_save.BonusWeight, 5);
	}
}

namespace RogueCastle
{
	// The song for the current screen (TitleScreen, LineageScreen and
	// SkillScreen PlayMusic; ProceduralLevelScreen.UpdateLevelSong: the
	// area of the hero's room, nothing in the courtyard). The death screen
	// plays its own stinger.
	void GameFlow::UpdateMusic()
	{
		switch (m_state)
		{
		case State::Title: Audio::PlayMusic("TitleScreenSong"); break;
		case State::Lineage:
		case State::Manor: Audio::PlayMusic("SkillTreeSong"); break;
		case State::Shop: break;
		case State::GameOver: break;
		case State::Castle:
		{
			if (m_level == nullptr || m_level->Player() == nullptr) break;
			float x = m_level->Player()->X(), y = m_level->Player()->Y();
			for (const Levels::PlacedRoom &r : m_level->Castle().rooms)
			{
				if (x < r.x || x >= r.Right() || y < r.y || y >= r.Bottom()) continue;
				switch (r.levelType)
				{
				case Levels::LevelType::NONE: Audio::StopMusic(); break;
				case Levels::LevelType::GARDEN: Audio::PlayMusic("GardenSong"); break;
				case Levels::LevelType::TOWER: Audio::PlayMusic("TowerSong"); break;
				case Levels::LevelType::DUNGEON: Audio::PlayMusic("DungeonSong"); break;
				default: Audio::PlayMusic("CastleSong"); break;
				}
				break;
			}
			break;
		}
		}
	}
}
