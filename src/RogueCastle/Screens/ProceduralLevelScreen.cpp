/* Port of RogueCastle/src/Screens/ProceduralLevelScreen.cs (subset). */
#include "ProceduralLevelScreen.h"
#include "../Types/EnemyType.h"
#include "../Types/TraitType.h"
#include "../EVs/GlobalEV.h"
#include "../Game.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/ProjectileObj.h"
#include "../GameObjects/ItemDropObj.h"
#include "../../DS2DEngine/CollisionMath.h"
#include "../../gfx.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../Audio/Audio.h"
#include "../../Text/Loc.h"
#include "../EVs/GameEV.h"
#include "../EVs/LevelEV.h"
#include "../EVs/SpellEVFuncs.h"
#include "../Types/ChestType.h"
#include "../GameObjects/ChestObj.h"
#include "../GameObjects/FairyChestObj.h"
#include "../Types/InputMapType.h"
#include "../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

ProceduralLevelScreen::ProceduralLevelScreen(const char *assets, uint32_t seed,
                                             std::vector<uint8_t> *chestFlags, int timesCastleBeaten)
	: m_chestFlags(chestFlags)
{
	/* LoadContent: the sheets this level needs (AGENTS.md: only the
	 * sheets the current area needs). */
	if (!SpriteSystem::SpriteLibrary::LoadSheet(assets, "playerSpritesheetBig"))
		return;
	/* The area decorations and breakables (asset-swap objects). */
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "castleTerrainSpritesheet");
	/* Area art (castle, tower, dungeon, garden) and the sky (RoomRenderer) */
	m_roomRenderer.LoadContent(assets);
	/* The enemies and their projectiles (the ported ones live in these). */
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "enemyFinalSpritesheetBig");
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "portraitSpritesheet");   /* the paintings in the portraits' frames */
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "enemyFinal2Spritesheet");
	/* The chests and their icons. */
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "miscSpritesheet");
	/* The spell projectiles (SpellDagger_Sprite & co). */
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "startingRoomSpritesheet");
	m_goldFont.Load(assets, "GoldFont");
	std::vector<Levels::RoomTemplate> templates;
	char path[512];
	snprintf(path, sizeof(path), "%s/rooms.txt", assets);
	if (!Levels::LoadRooms(path, templates))
	{
		fprintf(stderr, "cannot load %s\n", path);
		return;
	}

	/* Game.cs / ProceduralLevelScreen.cs line 196: gravity and terminal
	 * velocity. */
	m_physicsManager.SetGravity(0, -GlobalEV::GRAVITY);
	m_physicsManager.TerminalVelocity = 2000;

	/* Build the castle (LevelBuilder2). */
	Levels::LevelBuilder2 builder(templates);
	/* RL_SEED fixes the castle for testing (default: time based). */
	const char *seedEnv = getenv("RL_SEED");
	if (seedEnv != nullptr)
		seed = (uint32_t) strtoul(seedEnv, nullptr, 0);
	else if (seed == 0)
		seed = (uint32_t) std::chrono::steady_clock::now().time_since_epoch().count();
	for (int attempt = 0; attempt < 8; attempt++)
	{
		if (builder.CreateLevel(m_castle, Levels::DefaultAreas(), 4, seed + attempt))
			break;
		m_castle.rooms.clear();
	}
	if (m_castle.rooms.empty())
	{
		fprintf(stderr, "castle generation failed\n");
		return;
	}

	/* The player exists before the rooms: he is the enemies' target. */
	m_player = new PlayerObj();
	m_player->SetTerrain(&m_allTerrain);
	m_player->SetSlopes(&m_allSlopes);
	m_player->SetHazards(&m_allHazards);
	m_player->SetLevelScreen(this);
	m_physicsManager.AddObject(m_player);

	for (const Levels::PlacedRoom &placed : m_castle.rooms)
	{
		RoomObj *room = new RoomObj();
		room->LoadRoom(placed, m_player);
		m_roomList.push_back(room);
	}

	/* Every room's terrain lives in one world, so the player can walk
	 * between rooms through the door openings. */
	int roomLevel = 0;   /* ProceduralLevelScreen.cs line 861 */
	for (RoomObj *room : m_roomList)
	{
		for (TerrainObj *terrain : room->TerrainObjList())
		{
			m_physicsManager.AddObject(terrain);
			m_allTerrain.push_back(terrain);
		}
		for (const Slope &slope : room->SlopeList())
			m_allSlopes.push_back(slope);
		for (const Hazard &hazard : room->HazardList())
			m_allHazards.push_back(hazard);
		for (DS2DEngine::PhysicsObjContainer *obj : room->GameObjList())
			m_physicsManager.AddObject(obj);
		/* BreakableObj.cpp needs the level to drop its loot (no RTTI, so
		 * RoomObj keeps the breakables in their own list). */
		for (BreakableObj *breakable : room->BreakableObjList())
		{
			breakable->SetLevelScreen(this);
			m_allTerrain.push_back(breakable);   /* tables and chairs are floors too */
		}
		for (EnemyObj *enemy : room->EnemyList())
		{
			enemy->SetPlayerTarget(m_player);
			enemy->SetLevelScreen(this);
			enemy->SetTerrain(&m_allTerrain);
			m_physicsManager.AddObject(enemy);
		}
		for (ChestObj *chest : room->ChestList())
		{
			m_chestList.push_back(chest);
			chest->SetLevelScreen(this);
		}

		/* RoomObj.Level: the per-room counter with the area boost
		 * (ProceduralLevelScreen.cs lines 869-889). */
		int roomLevelMod = 0;
		switch (room->LevelType())
		{
		case Levels::LevelType::GARDEN:
			roomLevelMod = LevelEV::GARDEN_ROOM_LEVEL_BOOST - 2;
			break;
		case Levels::LevelType::TOWER:
			roomLevelMod = LevelEV::TOWER_ROOM_LEVEL_BOOST - 4;
			break;
		case Levels::LevelType::DUNGEON:
			roomLevelMod = LevelEV::DUNGEON_ROOM_LEVEL_BOOST - 6;
			break;
		default:
			roomLevelMod = LevelEV::CASTLE_ROOM_LEVEL_BOOST;
			break;
		}
		room->Level = roomLevel + roomLevelMod;
		if (timesCastleBeaten > 0)   /* New Game+ (ProceduralLevelScreen.cs line 885) */
			room->Level += LevelEV::NEWGAMEPLUS_LEVEL_BASE + (timesCastleBeaten - 1) * LevelEV::NEWGAMEPLUS_LEVEL_APPRECIATION;
		roomLevel++;
		/* ChestObj.Level (ProceduralLevelScreen.cs line 759); the
		 * RoomLevelDown rune grace is TODO with the runes. */
		for (ChestObj *chest : room->ChestList())
			chest->Level = room->Level / LevelEV::ROOM_LEVEL_MOD;
	}
	InitializeChests();

	/* InitializeEnemies (ProceduralLevelScreen.cs line 544): each enemy's
	 * level from its room (a boss room: the entrance it is linked to) */
	for (RoomObj *room : m_roomList)
	{
		bool boss = room->Name() == "Boss";
		int roomLevel = room->Level;
		if (boss && room->LinkedRoom() >= 0 && room->LinkedRoom() < (int) m_roomList.size())
			roomLevel = m_roomList[room->LinkedRoom()]->Level;
		for (EnemyObj *enemy : room->EnemyList())
			enemy->ApplyLevel(roomLevel / LevelEV::ROOM_LEVEL_MOD, boss);
	}

	m_currentRoom = m_roomList[0];
	RoomObj *start = m_roomList[0];
	m_player->X(start->X() + start->Width() * 0.5f);
	m_player->Y(start->Y() + start->Height() * 0.5f);
	/* Dev/test spawn override: RL_POS=x,y (world units). */
	const char *posEnv = getenv("RL_POS");
	if (posEnv != nullptr)
	{
		float px = 0, py = 0;
		if (sscanf(posEnv, "%f,%f", &px, &py) == 2)
		{
			m_player->X(px);
			m_player->Y(py);
		}
	}
	m_player->UpdateCollisionBoxes();

	/* Dev/test: RL_DEBUG_LOOT drops one of every loot type on the hero,
	 * to test ItemDropObj without hunting an enemy first. */
	if (getenv("RL_DEBUG_LOOT") != nullptr)
	{
		Vector2 pos(m_player->X(), m_player->Y() - 60);
		DropItem(pos, ItemDropType::Coin, ItemDropType::CoinAmount);
		DropItem(pos, ItemDropType::MoneyBag, ItemDropType::MoneyBagAmount);
		DropItem(pos, ItemDropType::Diamond, ItemDropType::DiamondAmount);
		DropItem(pos, ItemDropType::BigDiamond, ItemDropType::BigDiamondAmount);
		DropItem(pos, ItemDropType::Health, GameEV::ITEM_HEALTHDROP_AMOUNT);
		DropItem(pos, ItemDropType::Mana, GameEV::ITEM_MANADROP_AMOUNT);
		DropItem(pos, ItemDropType::Stat_Strength, 1);
		DropItem(pos, ItemDropType::Stat_Magic, 1);
		DropItem(pos, ItemDropType::Stat_Defense, 1);
	}

	if (getenv("RL_DEBUG_TRACE") != nullptr)
	{
		for (const Levels::PlacedRoom &r : m_castle.rooms)
		{
			fprintf(stderr, "room %s [%d] x=%.0f y=%.0f %dx%d doors:", r.name.c_str(),
			        (int) r.levelType, r.x, r.y, r.width, r.height);
			for (const Levels::Door &d : r.doors)
				fprintf(stderr, " %s(%.0f,%.0f)%s", Levels::DoorPosName(d.pos), d.x, d.y,
				        d.attached ? "" : " CLOSED");
			fprintf(stderr, "\n");
			for (const Levels::ObjData &o : r.objs)
			{
				if ((o.type == "ObjContainer" || o.type == "PhysicsObjContainer") && !o.sprite.empty())
					fprintf(stderr, "  obj %s%s (%.0f,%.0f)\n", o.sprite.c_str(),
					        o.breakable ? " [breakable]" : "", o.x, o.y);
				if (o.type == "EnemyObj")
					fprintf(stderr, "  enemy type=%d %s (%.0f,%.0f)\n", o.enemyType,
					        o.difficulty.c_str(), o.x, o.y);
			}
		}
	}

	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "physics list: %d objects, player tag=%d\n",
		        m_physicsManager.NumObjects(), m_player->CollisionTypeTag());
	m_valid = true;
}

void ProceduralLevelScreen::Update(GameTime *gameTime)
{
	if (!m_valid)
		return;

	/* ProceduralLevelScreen.cs line 1536: room, player, then physics.
	 * The time stop spell freezes the enemies (their logic, not the
	 * physics of the projectiles). */
	if (m_currentRoom != nullptr && !m_timeStopped)
		m_currentRoom->Update(gameTime);
	if (m_player != nullptr)
		m_player->Update(gameTime);

	/* The chests: fairy chest conditions, then the "press Up" arrow and
	 * the opening (PlayerObj.cs line 2131, ported as a bounds check). */
	bool upPressed = Game::GlobalInput.JustPressed(PLAYER_UP1)
	                  || Game::GlobalInput.JustPressed(PLAYER_UP2);
	Rectangle playerBounds = m_player != nullptr ? m_player->Bounds() : Rectangle();
	Rectangle roomBounds = m_currentRoom != nullptr ? m_currentRoom->Bounds() : Rectangle();
	for (ChestObj *chest : m_chestList)
	{
		/* FairyChestObj checks its condition from Draw, so only in the
		 * room the hero is in (ProceduralLevelScreen.cs: CurrentRoom.Draw) */
		bool here = roomBounds.Intersects(chest->Bounds());
		if (!chest->IsFairyChest() || here)
			chest->Update(gameTime);
		if (!here)
			continue;
		if (chest->IsOpen() || chest->IsLocked || m_player == nullptr)
			continue;
		if (chest->Bounds().Intersects(playerBounds) && m_player->IsTouchingGround()
		    && !m_player->ControlsLocked)
		{
			chest->ArrowVisible = true;
			if (upPressed)
			{
				chest->OpenChest(this, m_player);
				if (chest->IsOpen() && m_chestFlags != nullptr
				    && chest->ChestIndex < (int) m_chestFlags->size())
					(*m_chestFlags)[chest->ChestIndex] = 1;
			}
		}
	}

	/* m_projectileManager.Update(gameTime): move the projectiles and
	 * destroy the dead or out-of-play ones. */
	for (size_t i = 0; i < m_projectiles.size(); )
	{
		ProjectileObj *proj = m_projectiles[i];
		proj->Update(gameTime);
		if (getenv("RL_DEBUG_HITS") != nullptr)
			fprintf(stderr, "projupd: %s spell=%d x=%.0f y=%.0f killed=%d alive=%d\n",
			        proj->SpriteName().c_str(), (int) proj->Spell, proj->X(), proj->Y(),
			        (int) proj->IsKilled, (int) proj->IsAlive);
		Rectangle level = m_currentRoom != nullptr
		    ? m_currentRoom->Bounds()
		    : Rectangle(-100000, -100000, 200000, 200000);
		bool outOfPlay = proj->X() < level.X - 600 || proj->X() > level.Right() + 600
		              || proj->Y() > level.Bottom() + 600 || proj->Y() < level.Y - 1200;
		if (proj->IsKilled || (!proj->Held && !proj->IgnoreBoundsCheck && outOfPlay))
		{
			m_physicsManager.RemoveObject(proj);
			proj->IsAlive = false;
			delete proj;
			m_projectiles.erase(m_projectiles.begin() + i);
		}
		else
			i++;
	}

	/* Only simulate the objects near the current room (performance: the
	 * pair pass is O(n*n) and a castle has thousands of walls). */
	if (m_currentRoom != nullptr)
	{
		Rectangle b = m_currentRoom->Bounds();
		m_physicsManager.SetActiveBounds(Rectangle(b.X - 1320, b.Y - 720,
		                                          b.Width + 2 * 1320, b.Height + 2 * 720));
	}

	/* The loot (PlayerObj.cs line 2110: the pickup on contact). */
	for (size_t i = 0; i < m_itemDrops.size(); )
	{
		ItemDropObj *item = m_itemDrops[i];
		/* Update() runs the physics (the gravity pulls the drop down);
		 * the pickup check is a simple bounds overlap. */
		item->Update(gameTime);
		item->ObjContainer::Update(gameTime);
		if (item->CollectionCounter > 0)
			item->CollectionCounter -= (float) gameTime->ElapsedGameTime;
		bool collected = false;
		if (m_player != nullptr && item->IsCollectable())
		{
			Rectangle dropBounds = item->Bounds();
			Rectangle playerBounds = m_player->Bounds();
			if (dropBounds.Intersects(playerBounds))
			{
				item->GiveReward(m_player);
				if (!item->PopupText.empty())
				{
					uint32_t argb = ((uint32_t) item->PopupColour.A << 24)
					              | ((uint32_t) item->PopupColour.R << 16)
					              | ((uint32_t) item->PopupColour.G << 8)
					              | item->PopupColour.B;
					m_popups.push_back({item->PopupText, item->X(), (float) item->Bounds().Top(), 1.2f, argb});
				}
				Audio::PlaySound("CoinDrop1", "CoinDrop2", "CoinDrop3");
				collected = true;
			}
		}
		bool outOfPlay = false;
		if (m_currentRoom != nullptr)
		{
			Rectangle b2 = m_currentRoom->Bounds();
			outOfPlay = item->Y() > b2.Bottom() + 300;
		}
		if (collected || outOfPlay)
		{
			m_physicsManager.RemoveObject(item);
			delete item;
			m_itemDrops.erase(m_itemDrops.begin() + i);
		}
		else
			i++;
	}
	/* The floating pickup texts (TextManager.DisplayNumberStringText). */
	for (size_t i = 0; i < m_popups.size(); )
	{
		m_popups[i].timer -= (float) gameTime->ElapsedGameTime;
		m_popups[i].y -= (float) gameTime->ElapsedGameTime * 120;
		if (m_popups[i].timer <= 0)
			m_popups.erase(m_popups.begin() + i);
		else
			i++;
	}

	m_physicsManager.Update(gameTime);

	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "t: room=%s sprite=%s x=%.0f y=%.0f ground=%d\n",
		        m_currentRoom->Name().c_str(), m_player->SpriteName().c_str(), m_player->X(), m_player->Y(),
		        (int) m_player->m_isTouchingGround);

	/* Only check for room transitions if the player steps out of the
	 * camera zone (ProceduralLevelScreen.cs line 1547). */
	if (!CollisionMath::Intersects(Rectangle((int) m_player->X(), (int) m_player->Y(), 1, 1),
	                               m_camera.Bounds()))
		CheckForRoomTransition();

	UpdateCamera();
}

void ProceduralLevelScreen::InitializeChests()
{
	/* ProceduralLevelScreen.cs line 742: the chest colour roll and level,
	 * then the recentring (their anchors moved in the spritesheet). */
	if (m_chestFlags != nullptr && m_chestFlags->size() < m_chestList.size())
		m_chestFlags->resize(m_chestList.size(), 0);
	for (size_t i = 0; i < m_chestList.size(); i++)
	{
		ChestObj *chest = m_chestList[i];
		chest->ChestIndex = (int) i;

		if (chest->IsFairyChest())
		{
			((FairyChestObj *) chest)->SetConditionType();
		}
		else
		{
			/* Turning the chest into a brown, silver or gold chest
			 * (GameEV.CHEST_TYPE_CHANCE). */
			int chestRoll = rand() % 100 + 1, chestType = 0;
			for (int j = 0; j < 3; j++)
			{
				chestType += GameEV::CHEST_TYPE_CHANCE[j];
				if (chestRoll <= chestType)
				{
					if (j == 0)
						chest->SetChestType(ChestType::Brown);
					else if (j == 1)
						chest->SetChestType(ChestType::Silver);
					else
						chest->SetChestType(ChestType::Gold);
					break;
				}
			}
		}

		/* The room object coordinates are the chest's top-left corner
		 * (ProceduralLevelScreen.cs lines 804-806). */
		chest->X(chest->X() + chest->Width() * 0.5f);
		chest->Y(chest->Y() + 60);   /* the height of a tile */
		chest->UpdateCollisionBoxes();

		/* Chests opened in a previous run stay open (save.chestsOpened). */
		if (m_chestFlags != nullptr && i < m_chestFlags->size()
		    && (*m_chestFlags)[i] != 0)
			chest->ForceOpen();
	}

	if (getenv("RL_DEBUG_TRACE") != nullptr)
	{
		fprintf(stderr, "chests: %d\n", (int) m_chestList.size());
		for (ChestObj *chest : m_chestList)
			fprintf(stderr, "  chest %d type=%d level=%d fairy=%d (%.0f,%.0f)%s\n",
			        chest->ChestIndex, (int) chest->ChestType(), chest->Level,
			        (int) chest->IsFairyChest(), chest->X(), chest->Y(),
			        chest->IsOpen() ? " OPEN" : "");
	}
}

void ProceduralLevelScreen::DebugOpenChest(const char *arg)
{
	/* Dev/test: opens a chest to test the loot without walking to it:
	 * the chest index (default 0), optionally followed by "p" (force a
	 * blueprint) or "s" (force a stat drop). Called by GameFlow once the
	 * player stats are set (GivePrint/GiveStatDrop need them). */
	if (m_chestList.empty())
		return;
	int index = atoi(arg);
	if (index < 0 || index >= (int) m_chestList.size())
		index = 0;
	ChestObj *chest = m_chestList[index];
	size_t len = strlen(arg);
	if (len > 0 && arg[len - 1] == 'p')
		chest->ForcedItemType = ItemDropType::Blueprint;
	else if (len > 0 && arg[len - 1] == 's')
		chest->ForcedItemType = ItemDropType::Stat_Strength;
	chest->OpenChest(this, m_player);
	/* Count it as opened, like the real opening path does. */
	if (chest->IsOpen() && m_chestFlags != nullptr
	    && chest->ChestIndex < (int) m_chestFlags->size())
		(*m_chestFlags)[chest->ChestIndex] = 1;
}

void ProceduralLevelScreen::UpdateCamera()
{
	/* ProceduralLevelScreen.cs line 1621 (UpdateCamera). */
	if (m_player != nullptr)
	{
		m_camera.X(m_player->Position().X + GlobalEV::Camera_XOffset);
		m_camera.Y(m_player->Position().Y + GlobalEV::Camera_YOffset);
	}

	if (m_currentRoom != nullptr)
	{
		Rectangle cb = m_camera.Bounds();
		Rectangle rb = m_currentRoom->Bounds();

		/* Constrain the X axis of the camera to the current room. */
		if (cb.Width < m_currentRoom->Width())
		{
			if (cb.Left() < rb.Left())
				m_camera.X((float) (rb.Left() + cb.Width * 0.5f));
			else if (cb.Right() > rb.Right())
				m_camera.X((float) (rb.Right() - cb.Width * 0.5f));
		}
		else
			m_camera.X(m_currentRoom->X() + m_currentRoom->Width() * 0.5f);

		/* Constrain the Y axis of the camera to the current room. */
		if (cb.Height < m_currentRoom->Height())
		{
			if (cb.Top() < rb.Top())
				m_camera.Y((float) (rb.Top() + cb.Height * 0.5f));
			else if (cb.Bottom() > rb.Bottom())
				m_camera.Y((float) (rb.Bottom() - cb.Height * 0.5f));
		}
		else
			m_camera.Y(m_currentRoom->Y() + m_currentRoom->Height() * 0.5f);
	}
}

void ProceduralLevelScreen::CheckForRoomTransition()
{
	/* ProceduralLevelScreen.cs line 1189 (CheckForRoomTransition). */
	if (m_player == nullptr)
		return;
	for (RoomObj *roomObj : m_roomList)
	{
		if (roomObj != m_currentRoom && roomObj->Contains((int) m_player->X(), (int) m_player->Y()))
		{
			/* ProjectileManager: enemy projectiles do not survive room
			 * transitions (ProjectileObj DestroyOnRoomTransition). */
			/* Held projectiles (the flail ball, channel markers) are
			 * released by their owners; free only the loose ones. */
			for (size_t i = 0; i < m_projectiles.size(); )
			{
				ProjectileObj *proj = m_projectiles[i];
				if (proj->Held)
				{
					i++;
					continue;
				}
				m_physicsManager.RemoveObject(proj);
				proj->IsAlive = false;
				delete proj;
				m_projectiles.erase(m_projectiles.begin() + i);
			}
			if (m_currentRoom != nullptr)
			{
				m_currentRoom->ClearTempEnemies(m_physicsManager);
				for (EnemyObj *enemy : m_currentRoom->EnemyList())
					enemy->ResetState();
			}
			m_currentRoom = roomObj;
			for (EnemyObj *enemy : roomObj->EnemyList())
				enemy->ResetState();
			SpawnDementiaEnemy();
			m_player->RoomTransitionReset();
			if (getenv("RL_DEBUG_TRANSITIONS") != nullptr)
				fprintf(stderr, "room transition -> %s (%.0f, %.0f)\n",
				        roomObj->Name().c_str(), m_player->X(), m_player->Y());
			return;
		}
	}
}

void ProceduralLevelScreen::Draw(Camera2D *camera)
{
	if (!m_valid)
		return;

	/* The original look (sky, background, textured walls, borders,
	 * decorations) of the rooms in view, then their objects on top.
	 * m_roomList is built in the order of m_castle.rooms. */
	Rectangle view = camera->Bounds();
	for (size_t i = 0; i < m_castle.rooms.size() && i < m_roomList.size(); i++)
	{
		const Levels::PlacedRoom &r = m_castle.rooms[i];
		if (Rectangle((int) r.x, (int) r.y, r.width, r.height).Intersects(view))
			m_roomRenderer.Draw(camera, r, &m_roomList[i]->BGObjList());
	}
	for (RoomObj *room : m_roomList)
		room->Draw(camera);

	for (ProjectileObj *proj : m_projectiles)
		proj->Draw(camera);
	for (ItemDropObj *item : m_itemDrops)
		item->Draw(camera);
	/* The pickup texts, in screen space like every Text::Font draw. */
	for (const Popup &popup : m_popups)
	{
		float x = (popup.x - camera->X()) * 0.5f + 320;
		float y = (popup.y - camera->Y()) * 0.5f + 240;
		const char *text = popup.text.c_str();
		if (text[0] == 'L' && text[1] == 'O')
			text = Text::Loc::Get(popup.text.c_str());   /* the stat labels */
		float w = 0, h = 0;
		m_goldFont.Measure(text, w, h);
		m_goldFont.Draw(gfx_frame(), text, x - w * 0.5f * 0.7f, y, 0.7f, popup.colour,
		                Text::Align::Left);
	}
	if (m_player != nullptr)
	{
		m_player->Draw(camera);
		m_player->DrawSwearBubble(camera);
	}
}

void ProceduralLevelScreen::DropItem(Vector2 position, int dropType, float amount)
{
	/* ItemDropManager.cs line 37 (DropItem): pop from the pool, convert,
	 * fling it up. */
	ItemDropObj *item = new ItemDropObj();
	item->ConvertDrop(dropType, amount);
	m_itemDrops.push_back(item);
	m_physicsManager.AddObject(item);
	item->Position(position);
	/* ItemDropObj.ConvertDrop (line 87): a terrain box so it lands */
	item->UpdateCollisionBoxes();
	{
		Rectangle b = item->RelativeBounds();
		item->AddCollisionBox(b.X, b.Y, b.Width, b.Height, ::Consts::TERRAIN_HITBOX);
		item->DisableHitboxUpdatingFlag = true;
		item->UpdateTerrainBox();
	}
	item->AccelerationY(-(480 + (float) (rand() % 241)));          /* -720..-480 */
	item->AccelerationX((float) (rand() % 241) - 120);             /* -120..120 */
	item->AnimationDelay(1 / 20.0f);
	item->CollectionCounter = 0.2f;
}

void ProceduralLevelScreen::AddPopup(const std::string &text, float x, float y,
                                      uint32_t colour)
{
	m_popups.push_back({text, x, y, 1.2f, colour});
}

void ProceduralLevelScreen::DropItemWide(Vector2 position, int dropType, float amount)
{
	/* ItemDropManager.cs line 62 (DropItemWide): the player's own drops
	 * (Charon's toll) spread wider. */
	ItemDropObj *item = new ItemDropObj();
	item->ConvertDrop(dropType, amount);
	m_itemDrops.push_back(item);
	m_physicsManager.AddObject(item);
	item->Position(position);
	/* ItemDropObj.ConvertDrop (line 87): a terrain box so it lands */
	item->UpdateCollisionBoxes();
	{
		Rectangle b = item->RelativeBounds();
		item->AddCollisionBox(b.X, b.Y, b.Width, b.Height, ::Consts::TERRAIN_HITBOX);
		item->DisableHitboxUpdatingFlag = true;
		item->UpdateTerrainBox();
	}
	item->AccelerationY(-300 - (float) (rand() % 901));            /* -1200..-300 */
	item->AccelerationX((float) (rand() % 1201) - 600);             /* -600..600 */
	item->AnimationDelay(1 / 20.0f);
	item->CollectionCounter = 0.2f;
}

void ProceduralLevelScreen::AddProjectile(ProjectileObj *proj)
{
	m_projectiles.push_back(proj);
	m_physicsManager.AddObject(proj);
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "proj: %s (%.0f,%.0f) [add]\n",
		        proj->SpriteName().c_str(), proj->X(), proj->Y());
}

void ProceduralLevelScreen::FireProjectile(const char *spriteName, Vector2 position,
                                           Vector2 heading, float speed, int damage,
                                           Vector2 scale, bool weighted, EnemyObj *source,
                                           float lifeSpan, bool collidesWithTerrain)
{
	ProjectileObj *proj = new ProjectileObj(spriteName, position, heading, speed,
	                                        damage, scale, weighted);
	proj->Source = source;
	proj->Demented = source != nullptr && source->Demented;
	proj->LifeSpan = lifeSpan > 0 ? lifeSpan : 10;   /* ProjectileData.Lifespan = 10 */
	proj->DestroysWithEnemy = true;                 /* the C# default: dies on the hero */
	/* ProjectileManager.cs line 109: the sprite points along the shot */
	proj->Rotation(atan2f(heading.Y, heading.X) * 180.0f / (float) M_PI);
	/* CollidesWithTerrain: a terrain box, destroyed by the walls */
	if (collidesWithTerrain)
	{
		proj->CollidesWithTerrainFlag = true;
		proj->DestroysWithTerrain = true;
		proj->UpdateCollisionBoxes();
		Rectangle b = proj->RelativeBounds();
		proj->AddCollisionBox(b.X, b.Y, b.Width, b.Height, ::Consts::TERRAIN_HITBOX);
		proj->UpdateTerrainBox();
	}
	/* ProjectileManager.cs line 139: the launch speed goes into the
	 * acceleration, so the weighted ones (bones) fly in an arc */
	if (weighted)
	{
		proj->AccelerationX(heading.X * speed);
		proj->AccelerationY(heading.Y * speed);
		proj->RotationSpeed = 10;   /* the bone's ProjectileData */
	}
	m_projectiles.push_back(proj);
	m_physicsManager.AddObject(proj);
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "proj: %s (%.0f,%.0f) h=(%.2f,%.2f) speed=%.0f\n",
		        spriteName, position.X, position.Y, heading.X, heading.Y, speed);
}

ProjectileObj *ProceduralLevelScreen::FireSpellProjectile(const ProjectileData &data,
                                                           uint8_t spell)
{
	/* ProjectileManager.cs line 258 (FireProjectile), player source. */
	PlayerObj *source = data.source;
	float angle = data.angle.X + data.angleOffset;
	if (data.angle.X != data.angle.Y)
		angle = ((float) rand() / (float) RAND_MAX) * (data.angle.Y - data.angle.X)
		        + data.angle.X + data.angleOffset;
	bool flipped = source->Flip() == SpriteEffects::FlipHorizontally;
	if (flipped && source->Rotation() == 0)
		angle = 180 - angle;
	else if (flipped && source->Rotation() != 0)
		angle -= 180;
	float rad = angle * (float) M_PI / 180.0f;
	Vector2 heading(cosf(rad), sinf(rad));

	float x = flipped ? source->X() - data.sourceAnchor.X
	                  : source->X() + data.sourceAnchor.X;
	float y = source->Y() + data.sourceAnchor.Y;

	ProjectileObj *proj = new ProjectileObj(data.spriteName, Vector2(x, y),
	                                        heading, data.speed, data.damage,
	                                        data.scale, data.isWeighted);
	proj->SourcePlayer = source;
	proj->Spell = spell;
	proj->CollisionTypeTag(2);   /* CollisionType_PLAYER */
	proj->LifeSpan = data.lifespan != 0 ? data.lifespan : 10;   /* ProjectileLifeSpan */
	proj->Orientation = data.startingRotation * (float) M_PI / 180.0f;
	proj->CollidesWithTerrainFlag = data.collidesWithTerrain;
	proj->CollidesWith1Ways = data.collidesWith1Ways;
	proj->DestroysWithTerrain = data.destroysWithTerrain;
	proj->DestroysWithEnemy = data.destroysWithEnemy;
	proj->ChaseTarget = data.chaseTarget;
	proj->LockPosition = data.lockPosition;
	proj->Target = data.target;
	if (data.isWeighted)
		proj->Acceleration(heading * data.speed);
	if (!data.isCollidable)
		proj->IsCollidableFlag = false;

	/* The spells that touch walls get a terrain box over their bounds
	 * (only terrain-terrain box pairs are checked by the physics
	 * manager). */
	if (data.collidesWithTerrain || data.collidesWith1Ways)
	{
		proj->UpdateCollisionBoxes();
		Rectangle b = proj->RelativeBounds();
		proj->AddCollisionBox(b.X, b.Y, b.Width, b.Height, ::Consts::TERRAIN_HITBOX);
		proj->UpdateTerrainBox();
	}

	m_projectiles.push_back(proj);
	m_physicsManager.AddObject(proj);
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "spell: %s (%d) (%.0f,%.0f) angle=%.0f speed=%.0f dmg=%d\n",
		        data.spriteName.c_str(), (int) spell, x, y, angle, data.speed,
		        data.damage);
	return proj;
}

void ProceduralLevelScreen::DamageAllEnemies(int damage)
{
	/* ProjectileObj.cs line 581: the nuke hits every enemy of the room. */
	if (m_currentRoom == nullptr)
		return;
	for (EnemyObj *enemy : m_currentRoom->EnemyList())
	{
		if (!enemy->IsKilled && !enemy->NonKillable)
			enemy->HitEnemy(damage, Vector2(enemy->X(), enemy->Y()), true);
	}
}

void ProceduralLevelScreen::CastTimeStop()
{
	/* The stop watch: the enemies freeze while the spell is up (the
	 * hero's mana pays TIMESTOP_ACTIVE_MANA_DRAIN per tick). */
	m_timeStopped = true;
	Audio::PlaySound("Cast_TimeStop");
}

void ProceduralLevelScreen::StopTimeStop()
{
	m_timeStopped = false;
}

} // namespace RogueCastle

namespace RogueCastle
{
void ProceduralLevelScreen::SpawnDementiaEnemy()
{
	/* ProceduralLevelScreen.cs line 1382: entering an ordinary room, a 20%
	 * chance of an imaginary enemy where a real one stands (the flying or
	 * walking kind like it), harmless and unkillable */
	const RoomObj *room = m_currentRoom;
	if (room == nullptr || m_player == nullptr || m_player->Stats() == nullptr
	    || !m_player->Stats()->hero.HasTrait(TraitType::Dementia))
		return;
	static const char *const skip[] = { "Start", "Tutorial", "Ending", "CastleEntrance", "Bonus", "Throne", "Secret", "Boss", "ChallengeBoss" };
	for (const char *name : skip)
		if (room->Name() == name) return;
	/* Dev/test: RL_DEBUG_DEMENTIA=1 spawns one in every room */
	bool always = getenv("RL_DEBUG_DEMENTIA") != nullptr;
	if (room->LevelType() == Levels::LevelType::NONE || (!always && rand() % 1000 >= (int) (GameEV::TRAIT_DEMENTIA_SPAWN_CHANCE * 1000)))
		return;
	std::vector<EnemyObj *> candidates;
	for (EnemyObj *e : room->EnemyList())
		if (e != nullptr && !e->IsKilled && e->Type != EnemyType::Turret && e->Type != EnemyType::SpikeTrap
		    && e->Type != EnemyType::Portrait && e->Type != EnemyType::Eyeball && e->Type != EnemyType::Starburst)
			candidates.push_back(e);
	if (candidates.empty())
		return;
	EnemyObj *model = candidates[rand() % candidates.size()];
	uint8_t type = model->IsWeightedFlag
		? LevelEV::DEMENTIA_GROUND_LIST[rand() % (sizeof(LevelEV::DEMENTIA_GROUND_LIST))]
		: LevelEV::DEMENTIA_FLIGHT_LIST[rand() % (sizeof(LevelEV::DEMENTIA_FLIGHT_LIST))];
	EnemyObj *enemy = m_currentRoom->SpawnTempEnemy(type, model->Position(), m_player);
	if (enemy == nullptr)
		return;
	enemy->Demented = true;
	enemy->NonKillable = true;
	enemy->SetPlayerTarget(m_player);
	enemy->SetLevelScreen(this);
	enemy->SetTerrain(&m_allTerrain);
	m_physicsManager.AddObject(enemy);
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "dementia: enemy %d in %s\n", (int) type, room->Name().c_str());
}
}
