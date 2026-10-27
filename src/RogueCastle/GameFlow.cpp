#include "GameFlow.h"

#include "../Text/Loc.h"
#include "../platform.h"
#include "Game.h"

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
		bool ok = m_lineage.LoadContent(assetsDir) && m_manor.LoadContent(assetsDir) && m_hud.LoadContent(assetsDir);
		Save::Read(m_save, savePath);
		// A saved hero still alive goes straight back to the castle
		if (!m_save.hero.name.empty() && !m_save.IsDead) StartCastle();
		else
		{
			m_state = State::Lineage;
			m_lineage.Open(m_save, m_generator);
		}
		return ok;
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
		m_state = State::Manor;
		m_manor.Open(m_save);
	}

	void GameFlow::Update(uint32_t buttons, double dt)
	{
		uint32_t pressed = buttons & ~m_previous;
		m_previous = buttons;
		switch (m_state)
		{
		case State::Lineage:
			if (m_lineage.Update(buttons, pressed, dt)) StartCastle();
			break;
		case State::Manor:
			if (m_manor.Update(buttons, pressed, dt))
			{
				Save();
				m_state = State::Lineage;
				m_lineage.Open(m_save, m_generator);
			}
			break;
		case State::Castle:
		{
			GameTime gameTime(dt);
			Game::GlobalInput.Update(buttons);
			m_level->Update(&gameTime);
			// Test stand-in until the hero has health: SELECT + START = death
			if ((buttons & (BTN_SELECT | BTN_START)) == (BTN_SELECT | BTN_START) && (pressed & (BTN_SELECT | BTN_START)))
				OnPlayerDeath();
			break;
		}
		}
	}

	void GameFlow::Draw(uint32_t *frame, double drawElapsed)
	{
		switch (m_state)
		{
		case State::Lineage:
			m_lineage.Draw(frame);
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
			HUDValues v;
			v.maxHealth = 100; // TODO when the hero has stats: PlayerObj health/mana
			v.health = 100;
			v.maxMana = 100;
			v.mana = 100;
			v.gold = m_save.Gold;
			v.level = m_save.skills.PlayerLevel();
			v.heroName = Text::Loc::Format(m_save.hero.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
				m_save.hero.name, m_save.hero.romanNumeral);
			m_hud.Draw(frame, v);
			break;
		}
		}
	}
}
