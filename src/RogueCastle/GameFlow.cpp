#include "GameFlow.h"

#include "../Text/Loc.h"
#include "../Text/LocIDs.h"
#include "../platform.h"
#include "Game.h"

#include <cstdlib>
#include <ctime>

namespace RogueCastle
{
	bool GameFlow::Init(const char *assetsDir, const char *savePath)
	{
		m_assets = assetsDir;
		m_savePath = savePath;
		Text::Loc::Load(assetsDir, "fr");
		m_generator = Lineage::Generator((uint32_t) time(nullptr));
		std::string names = m_assets + "/HeroNames.txt", female = m_assets + "/HeroineNames.txt";
		m_generator.LoadNames(names.c_str(), female.c_str());
		bool ok = m_lineage.LoadContent(assetsDir) && m_manor.LoadContent(assetsDir) && m_hud.LoadContent(assetsDir)
			&& m_map.LoadContent(assetsDir) && m_pause.LoadContent(assetsDir)
			&& m_court.LoadContent(assetsDir) && m_smith.LoadContent(assetsDir) && m_enchant.LoadContent(assetsDir);
		Save::Read(m_save, savePath);
		if (getenv("RL_DEBUG_LEVEL") == nullptr)
		{
			// InitializeStartingText: continue / choose an heir / new legacy
			const char *text = m_save.hero.name.empty() ? "LOC_ID_TITLE_SCREEN_4"
				: m_save.IsDead ? "LOC_ID_TITLE_SCREEN_3" : "LOC_ID_TITLE_SCREEN_2";
			ok = m_title.Open(assetsDir, text) && ok;
			m_state = State::Title;
			return ok;
		}
		LeaveTitle();
		return ok;
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
		Save::Write(m_save, m_savePath.c_str());
	}

	void GameFlow::StartCastle()
	{
		delete m_level;
		if (!m_save.castleExists)
		{
			m_save.castleExists = true;
			m_save.castleSeed = (uint32_t) time(nullptr);
			m_save.roomsVisited.clear();
			m_save.chestsOpened.clear();
		}
		m_save.IsDead = false;
		m_level = new ProceduralLevelScreen(m_assets.c_str(), m_save.castleSeed);
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
		m_save.castleExists = false; // a new castle for the next heir (unless the architect locks it)
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
		switch (m_state)
		{
		case State::Lineage:
			/* The new heir starts in the courtyard (the castle's first room) */
			if (m_lineage.Update(buttons, pressed, dt)) StartCastle();
			break;
		case State::Title:
			if (m_title.Update(pressed, dt)) LeaveTitle();
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
			if (m_paused)
			{
				m_paused = m_pause.Update(pressed);
				break;
			}
			if (m_mapOpen)
			{
				m_mapOpen = m_map.Update(buttons, pressed, dt);
				break;
			}
			PlayerObj *player = m_level->Player();
			if (player != nullptr && !player->IsKilled && (pressed & BTN_START) && !(buttons & BTN_SELECT))
			{
				m_paused = true;
				break;
			}
			if (player != nullptr && (pressed & BTN_SELECT) && !(buttons & BTN_START))
			{
				m_map.Open(player->X(), player->Y());
				m_mapOpen = true;
				break;
			}
			if (player != nullptr && InCourtyard())
			{
				StartingRoom::Action action = m_court.Update(player, m_save.skills, pressed, dt);
				if (action == StartingRoom::Action::Blacksmith || action == StartingRoom::Action::Enchantress)
				{
					m_shop = action == StartingRoom::Action::Blacksmith ? &m_smith : &m_enchant;
					m_shop->Open(m_save, MaxWeight());
					m_state = State::Shop;
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
				v.maxHealth = (int) m_level->Player()->MaxHealth;
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
