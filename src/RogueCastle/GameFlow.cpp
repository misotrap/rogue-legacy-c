#include "GameFlow.h"
#include "../perf.h"

#include "../Audio/Audio.h"
#include "../Text/Loc.h"
#include "../Text/LocIDs.h"
#include "../platform.h"
#include "Game.h"
#include "EVs/GameEV.h"
#include "Types/TraitType.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
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
			&& m_map.LoadContent(assetsDir) && m_pause.LoadContent(assetsDir) && m_getItem.LoadContent(assetsDir) && m_bossRoom.LoadContent(assetsDir) && m_credits.LoadContent(assetsDir)
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

	void GameFlow::Save(bool vitals)
	{
		/* The hero's vitals persist between runs (PlayerStats); not once
		 * the hero is gone (AddFamilyNode reset them for the next heir) */
		if (vitals && m_level != nullptr && m_level->Player() != nullptr && m_state == State::Castle)
		{
			m_save.CurrentHealth = (int) m_level->Player()->CurrentHealth;
			m_save.CurrentMana = (int) m_level->Player()->CurrentMana;
		}
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
		m_level = new ProceduralLevelScreen(m_assets.c_str(), m_save.castleSeed,
		                                     &m_save.chestsOpened, m_save.TimesCastleBeaten);
		/* The hero's real stats (Game.PlayerStats) drive the player. */
		m_level->Player()->SetPlayerStats(&m_save);
		/* The trait scale and speed (ProceduralLevelScreen.cs OnEnter,
		 * lines 2953-2975). */
		{
			PlayerObj *player = m_level->Player();
			Vector2 scale(2, 2);
			if (m_save.hero.HasTrait(TraitType::Gigantism))
				scale = Vector2(GameEV::TRAIT_GIGANTISM, GameEV::TRAIT_GIGANTISM);
			else if (m_save.hero.HasTrait(TraitType::Dwarfism))
				scale = Vector2(GameEV::TRAIT_DWARFISM, GameEV::TRAIT_DWARFISM);
			if (m_save.hero.HasTrait(TraitType::Ectomorph))
			{
				scale.X *= 0.825f;
				scale.Y *= 1.15f;
			}
			else if (m_save.hero.HasTrait(TraitType::Endomorph))
			{
				scale.X *= 1.25f;
				scale.Y *= 1.175f;
			}
			player->Scale(scale);
			if (m_save.hero.HasTrait(TraitType::Hyperactive))
				player->Speed *= 1 + GameEV::TRAIT_MOVESPEED_AMOUNT;
		}
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
		/* The mana persists like the health (PlayerStats.CurrentMana). */
		float maxMana = m_level->Player()->MaxMana();
		m_level->Player()->CurrentMana =
			m_save.CurrentMana > 0 && m_save.CurrentMana <= maxMana
			? (float) m_save.CurrentMana : maxMana;
		/* Dev/test: RL_DEBUG_TRAIT=<t1>[,<t2>] from the start (also below) */
		if (const char *debugTrait = getenv("RL_DEBUG_TRAIT"))
		{
			m_save.hero.trait1 = (uint8_t) atoi(debugTrait);
			const char *comma = strchr(debugTrait, ',');
			m_save.hero.trait2 = comma != nullptr ? (uint8_t) atoi(comma + 1) : 0;
		}
		/* Dev/test: RL_DEBUG_CHEST opens a chest (index, optional p/s). */
		{ GetItem::Reward r; while (GetItem::Pop(r)) {} }
		const char *debugChest = getenv("RL_DEBUG_CHEST");
		if (debugChest != nullptr)
			m_level->DebugOpenChest(debugChest);
		m_state = State::Castle;
		m_mapOpen = false;
		m_paused = false;
		m_getItemOpen = false;
		m_bossRoom.Reset();
		m_map.Reset(m_level->Castle().rooms.size());
		/* LoadingScreen.cs line 256: a hero who already went in resumes at
		 * the castle entrance, gate closed, not in the courtyard */
		if (!m_save.LoadStartingRoom && m_level->Player() != nullptr)
			for (RoomObj *r : m_level->RoomList())
				if (r->Name() == "CastleEntrance")
				{
					m_level->Player()->X(r->X() + 300);
					m_level->Player()->Y(r->Y() + r->Height() - 300);
					m_bossRoom.ForceGateClosed();
					break;
				}
		Save();
	}

	// ProceduralLevelScreen death -> GameOverScreen -> SkillScreen
	void GameFlow::AddFamilyNode(bool beatenABoss)
	{
		// The next heir starts with full health and mana (0 = the maximum)
		m_save.CurrentHealth = 0;
		m_save.CurrentMana = 0;
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
		node.beatenABoss = beatenABoss;
		m_save.familyTree.push_back(node);
	}

	void GameFlow::OnCastleBeaten()
	{
		AddFamilyNode(true);
		m_save.LoadStartingRoom = true;   /* EnemyObj_LastBoss.cs line 1698 */
		m_save.NewBossBeaten = false;
		m_save.NumEnemiesBeaten = 0;
		m_save.LastbossBeaten = true;
		m_save.TimesCastleBeaten++;
		// TitleScreen (m_startNewGamePlus): the bosses and the castle anew
		m_save.EyeballBossBeaten = m_save.FairyBossBeaten = false;
		m_save.BlobBossBeaten = m_save.FireballBossBeaten = false;
		m_save.castleExists = false;
		m_save.lastCastleSeed = 0;
		m_save.LockCastle = false;
		m_save.HasArchitectFee = false;
		Save(false);
		delete m_level;
		m_level = nullptr;
		m_credits.Open((int) m_save.familyTree.size());
		m_state = State::Credits;
	}

	void GameFlow::OnPlayerDeath()
	{
		AddFamilyNode(m_save.NewBossBeaten);
		m_save.LoadStartingRoom = true;   /* GameOverScreen.cs line 182 */
		/* GameOverScreen.cs lines 177-192: the Lich's gains go with him;
		 * the faceless portraits if he had Prosopagnosia */
		m_save.LichHealth = 0;
		m_save.LichMana = 0;
		m_save.LichHealthMod = 1;
		m_save.HasProsopagnosia = m_save.hero.HasTrait(TraitType::Prosopagnosia);
		const Lineage::Heir &h = m_save.hero;
		m_save.IsDead = true;
		m_save.TimesDead++;
		m_save.NumEnemiesBeaten = 0;
		m_save.NewBossBeaten = false;
		/* A new castle for the next heir; the architect may bring this one
		 * back from the courtyard (StartingRoomObj.ActivateArchitect) */
		m_save.lastCastleSeed = m_save.castleSeed;
		m_save.lastChestsOpened = m_save.chestsOpened;
		m_save.castleExists = false;
		m_save.LockCastle = false;
		m_save.HasArchitectFee = false;
		Save(false);

		// GameOverScreen draws the hero itself, lit under the spotlight:
		// its look and place, and the last frame without it
		HeroLook look;
		int killerKind = 0;
		std::string killerName;
		if (PlayerObj *p = m_level->Player())
		{
			killerKind = p->KillerKind;
			killerName = p->KillerName;
			for (int i = 0; i < p->NumChildren(); i++)
			{
				look.colours.push_back(p->GetChildAt(i)->TextureColor());
				look.visible.push_back(p->GetChildAt(i)->Visible());
			}
			const int EXTRA = 15;
			std::string base = p->SpriteName(), extra = p->NumChildren() > EXTRA ? p->GetChildAt(EXTRA)->SpriteName() : "";
			base = base.substr(0, base.find("_Character"));
			if (p->NumChildren() > EXTRA && p->GetChildAt(EXTRA)->Visible() && extra.compare(0, base.size(), base) == 0)
				look.extraSuffix = extra.substr(base.size());
			look.flip = p->Flip() == SpriteEffects::FlipHorizontally;
			look.scale = p->ScaleX();
			look.x = p->X() - m_level->Camera()->X() + 660;
			look.y = p->Y() - m_level->Camera()->Y() + 360;
			look.feet = p->Bounds().Bottom() - p->Y();
			p->Visible(false);
			p->Opacity(1);
			m_lastFrame.assign((size_t) SCREEN_W * SCREEN_H, 0xFF000000u);
			DrawState(m_lastFrame.data(), 0);
		}
		delete m_level;
		m_level = nullptr;
		m_deathTimer = 0;
		std::string name = Text::Loc::Format(h.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
			h.name, h.romanNumeral);
		m_gameOver.Open(m_assets.c_str(), m_lastFrame, name, h.isFemale, look, killerKind, killerName);
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
			if (m_lineage.Update(buttons, pressed, dt))
			{
				/* LoadingScreen.cs line 345: the new heir starts in the manor
				 * (the skill screen), then walks into the courtyard */
				StartCastle();
				Save();
				m_state = State::Manor;
				m_manor.Open(m_save);
			}
			break;
		case State::Title:
			if (pressed & BTN_SELECT)
			{
				m_options.Open(false);
				m_optionsOpen = true;
				break;
			}
			if (pressed & BTN_Y)
			{
				// The credits icon (TitleScreen: MENU_CREDITS)
				m_title.Close();
				m_credits.Open((int) m_save.familyTree.size(), true);
				m_creditsFromTitle = true;
				m_state = State::Credits;
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
		case State::Credits:
			if (m_credits.Update(pressed, dt))
			{
				if (m_creditsFromTitle)
				{
					m_creditsFromTitle = false;
					OpenTitle();
					break;
				}
				m_state = State::Lineage;
				m_lineage.Open(m_save, m_generator);
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
					/* StartingRoomObj.OnEnter (line 252): full health and mana,
					 * with what was just bought */
					PlayerObj *p = m_level->Player();
					p->CurrentHealth = p->MaxHealth();
					p->CurrentMana = p->MaxMana();
					p->SetPlayerStats(&m_save);   // the class pieces, colours and runes
					StartingRoom::PlaceAtLeftEdge(p);
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
						/* the castle the last heir died in, rebuilt around the
						 * hero, who stays where he is in the courtyard; this run
						 * pays the fee (LoadingScreen.cs line 234) */
						m_save.LockCastle = true;
						m_save.HasArchitectFee = true;
						m_save.ArchitectUsed = true;
						m_save.castleSeed = m_save.lastCastleSeed;
						m_save.chestsOpened = m_save.lastChestsOpened;
						m_save.castleExists = true;
						Vector2 at = m_level->Player()->Position();
						bool tollVisible = m_court.TollVisible();
						StartCastle();
						m_court.ResetToll(tollVisible);
						m_level->Player()->Position(at);
						Save();
						Audio::PlaySound("Architect_Lever");
						m_dialogue.Open("Castle Lock Complete Architect", m_save.hero.isFemale);
					}
				}
				break;
			}
			if (m_getItemOpen)
			{
				m_getItemOpen = m_getItem.Update(pressed, dt);
				break;
			}
			GetItem::Reward reward;
			if (GetItem::Pop(reward))
			{
				const Vector2 cam = m_level->Camera()->Position();
				m_getItem.Open(reward, cam.X, cam.Y, m_save.TimesCastleBeaten);
				m_getItemOpen = true;
				Save();
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
				/* StartingRoomObj.cs line 346: the roofs of the stands are
				 * solid only once the stand is there */
				if (!m_level->RoomList().empty())
				{
					RoomObj *start = m_level->RoomList()[0];
					const std::pair<const char *, Skills::SkillType> blocks[] = { { "BlacksmithBlock", Skills::SkillType::Smithy },
						{ "EnchantressBlock", Skills::SkillType::Enchanter }, { "ArchitectBlock", Skills::SkillType::Architect } };
					for (const auto &b : blocks)
						if (TerrainObj *t = start->TerrainNamed(b.first))
							static_cast<DS2DEngine::GameObj *>(t)->Visible(m_save.skills.Level(b.second) > 0);
				}
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
					if (m_save.lastCastleSeed == 0)
						m_dialogue.Open("No Castle Architect", m_save.hero.isFemale);   // no castle to keep yet
					else if (m_save.LockCastle)
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
			bool blocked = m_bossRoom.Update(m_level, m_save, pressed, dt, m_getItemOpen);
			if (const char *talk = m_bossRoom.TakeDialogue())
				m_dialogue.Open(talk, m_save.hero.isFemale);
			if (m_bossRoom.LastBossBeaten())
			{
				OnCastleBeaten();
				break;
			}
			GameTime gameTime(dt);
			Game::GlobalInput.Update(blocked ? 0 : buttons);
			if (!m_bossRoom.Cutscene()) m_level->Update(&gameTime);
			/* Clonus (ProceduralLevelScreen.cs line 1603): every 12-35 s,
			 * the screen shakes for 1-1.5 s (ShakeScreen(1), UpdateShake) */
			if (m_save.hero.HasTrait(TraitType::Clonus))
			{
				auto randomFloat = [](float a, float b) { return a + (b - a) * (float) rand() / (float) RAND_MAX; };
				if (m_clonusTimer <= 0 && m_shakeTimer <= 0) m_clonusTimer = randomFloat(GameEV::TRAIT_CLONUS_MIN, GameEV::TRAIT_CLONUS_MAX);
				m_clonusTimer -= (float) dt;
				if (m_clonusTimer <= 0 && m_shakeTimer <= 0)
					m_shakeTimer = randomFloat(1, 1.5f);
				if (m_shakeTimer > 0)
				{
					m_shakeTimer -= (float) dt;
					DS2DEngine::Camera2D *cam = m_level->Camera();
					cam->X(cam->X() + (rand() % 2 ? 1 : -1) * randomFloat(0, 1) * 2);
					cam->Y(cam->Y() + (rand() % 2 ? 1 : -1) * randomFloat(0, 1) * 2);
				}
			}
			/* Dev/test: RL_DEBUG_SPELL=<SpellType> casts that spell once
			 * after the first update (so the current room is known);
			 * the hero's spell is overridden. */
			static const char *debugSpell = getenv("RL_DEBUG_SPELL");
			static bool spellCast = false;
			if (debugSpell != nullptr && !spellCast && m_level->Player() != nullptr)
			{
				spellCast = true;
				m_save.hero.spell = (uint8_t) atoi(debugSpell);
				m_level->Player()->CastSpell(false);
			}
			/* Dev/test: RL_DEBUG_CLASS=<ClassType> + RL_DEBUG_POWER=1
			 * fires that class power once. */
			static const char *debugClass = getenv("RL_DEBUG_CLASS");
			static const char *debugPower = getenv("RL_DEBUG_POWER");
			static const char *debugTrait = getenv("RL_DEBUG_TRAIT");
			static bool powerCast = false;
			if ((debugClass != nullptr || debugTrait != nullptr) && !powerCast
			    && m_level->Player() != nullptr)
			{
				powerCast = true;
				if (debugClass != nullptr)
					m_save.hero.classType = (uint8_t) atoi(debugClass);
				if (debugTrait != nullptr)
				{
					/* "trait1" or "trait1,trait2". */
					m_save.hero.trait1 = (uint8_t) atoi(debugTrait);
					const char *comma = strchr(debugTrait, ',');
					m_save.hero.trait2 = comma != nullptr
					                     ? (uint8_t) atoi(comma + 1) : 0;
				}
				if (debugPower != nullptr)
					m_level->Player()->CastClassPower();
			}
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
		if (m_state != State::Castle) perf_label("menu");
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
		case State::Credits:
			m_credits.Draw(frame);
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
			m_bossRoom.DrawWorld(m_level->Camera());
			perf_mark(PERF_WORLD);
			if (m_level->CurrentRoom() != nullptr)
			{
				static const char *const AREAS[] = { "cour", "chateau", "jardin", "donjon", "tour" };
				perf_label(InCourtyard() ? "cour" : AREAS[(int) m_level->CurrentRoom()->LevelType() % 5]);
			}
			const bool glasses = m_save.SpecialItem == 8;   // SpecialItemType.Glasses
			if (m_level->Player() != nullptr)
			{
				DS2DEngine::Camera2D *cam = m_level->Camera();
				bool dungeon = m_level->CurrentRoom() != nullptr && m_level->CurrentRoom()->LevelType() == Levels::LevelType::DUNGEON;
				m_traitEffects.World(frame, m_save.hero, glasses, dungeon,
					(m_level->Player()->X() - cam->X()) * 0.5f + SCREEN_W / 2, (m_level->Player()->Y() - cam->Y()) * 0.5f + SCREEN_H / 2);
			}
			perf_mark(PERF_EFFECTS);
			HUDValues v;
			/* The hero's real vitals (PlayerObj health and mana). */
			if (m_level->Player() != nullptr)
			{
				v.maxHealth = (int) m_level->Player()->MaxHealth();
				v.health = (int) m_level->Player()->CurrentHealth;
				v.maxMana = (int) m_level->Player()->MaxMana();
				v.mana = (int) m_level->Player()->CurrentMana;
			}
			v.gold = m_save.Gold;
			v.cip = m_save.hero.HasTrait(TraitType::CIP);
			v.level = m_save.skills.PlayerLevel();
			v.heroName = Text::Loc::Format(m_save.hero.isFemale ? "LOC_ID_LINEAGE_OBJ_14_NEW" : "LOC_ID_LINEAGE_OBJ_12_NEW",
				m_save.hero.name, m_save.hero.romanNumeral);
			m_hud.Draw(frame, v);
			if (m_level->Player() != nullptr)
				m_map.Visit(m_level->Castle(), m_level->Player()->X(), m_level->Player()->Y());
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
			perf_mark(PERF_HUD);
			m_traitEffects.Screen(frame, m_save.hero, glasses, m_level->Camera(), drawElapsed);
			perf_mark(PERF_EFFECTS);
			m_dialogue.Dyslexia = m_save.hero.HasTrait(TraitType::Dyslexia);
			if (m_dialogue.IsOpen()) m_dialogue.Draw(frame);
			m_bossRoom.Draw(frame);
			if (m_getItemOpen) m_getItem.Draw(frame);
			if (m_mapOpen) m_map.Draw(frame, m_level->Castle(), m_save.hero.HasTrait(TraitType::Alzheimers));
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
		case State::Credits: break;
		case State::Castle:
		{
			if (m_level == nullptr || m_level->Player() == nullptr) break;
			float x = m_level->Player()->X(), y = m_level->Player()->Y();
			for (const Levels::PlacedRoom &r : m_level->Castle().rooms)
			{
				if (x < r.x || x >= r.Right() || y < r.y || y >= r.Bottom()) continue;
				if (r.name == "Boss")
				{
					const char *song = m_bossRoom.Music();
					if (*song) Audio::PlayMusic(song); else Audio::StopMusic();
					break;
				}
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
