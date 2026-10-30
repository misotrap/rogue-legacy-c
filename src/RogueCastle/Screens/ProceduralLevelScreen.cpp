/* Port of RogueCastle/src/Screens/ProceduralLevelScreen.cs (subset). */
#include "ProceduralLevelScreen.h"
#include "../../debugflags.h"
#include "../Types/EnemyType.h"
#include "../Types/TraitType.h"
#include "../EVs/GlobalEV.h"
#include "../Game.h"
#include "../GameObjects/EnemyObjs/EnemyObj_Platform.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/ProjectileObj.h"
#include "../GameObjects/TeleporterObj.h"
#include "../GameObjects/ItemDropObj.h"
#include "../../DS2DEngine/CollisionMath.h"
#include "../../gfx.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../Audio/Audio.h"
#include "../../Text/Loc.h"
#include "../Dialogue/DialogueData.h"
#include "../EVs/GameEV.h"
#include "../EVs/LevelEV.h"
#include "../EVs/SpellEVFuncs.h"
#include "../Types/ChestType.h"
#include "../Types/ChestConditionType.h"
#include "../GameObjects/ChestObj.h"
#include "../GameObjects/FairyChestObj.h"
#include "../Types/InputMapType.h"
#include "../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

ProceduralLevelScreen::ProceduralLevelScreen(const char *assets, uint32_t seed,
                                             std::vector<uint8_t> *chestFlags, int timesCastleBeaten,
                                             int *diaryEntry)
	: m_diaryEntry(diaryEntry), m_chestFlags(chestFlags)
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
	/* The objective plate's font (Junicode, like the C#) and its box;
	 * the DialogBox sprite lives in the misc sheet the chests load. */
	m_objFont.Load(assets, "Junicode");
	m_objPlate = new DS2DEngine::SpriteObj("DialogBox_Sprite");
	m_objPlate->ForceDraw(true);
	m_objCamera.Position(Vector2(660, 360));
	m_objCamera.Zoom(0.5f);
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
			m_castleTerrain.push_back(terrain);
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
			m_castleTerrain.push_back(breakable);   /* tables and chairs are floors too */
		}
		for (EnemyObj *enemy : room->EnemyList())
		{
			enemy->SetPlayerTarget(m_player);
			enemy->SetLevelScreen(this);
			enemy->SetTerrain(&m_allTerrain);
			m_physicsManager.AddObject(enemy);
			if (enemy->Type == EnemyType::Platform)
			{
				/* the folding platforms' floors, there once unfolded */
				PlatformFloor *floor = static_cast<EnemyObj_Platform *>(enemy)->Floor();
				floor->Populate((int) enemy->X(), (int) enemy->Y(), 1, 1, true, false, false, false);
				m_physicsManager.AddObject(floor);
				m_castleTerrain.push_back(floor);
			}
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

	/* The teleporters: one per Linker (the area junction), EntranceBoss
	 * and the CastleEntrance (LevelBuilder2.cs lines 186-203 and
	 * CastleEntranceRoomObj.cs line 43), on the floor tile of the room's
	 * centre. Bounds/Anchor are read before the position is set, like
	 * the C# formula. */
	for (size_t i = 0; i < m_roomList.size() && i < m_castle.rooms.size(); i++)
	{
		const char *n = m_castle.rooms[i].name.c_str();
		if (strcmp(n, "Linker") != 0 && strcmp(n, "EntranceBoss") != 0 && strcmp(n, "CastleEntrance") != 0)
			continue;
		RoomObj *room = m_roomList[i];
		TeleporterObj *teleporter = new TeleporterObj();
		if (strcmp(n, "CastleEntrance") == 0)
			/* CastleEntranceRoomObj.cs line 109: its own spot, left of the
			 * closed gate (the room's centre is behind it). */
			teleporter->Position(Vector2(room->X() + room->Width() * 0.5f - 600,
				room->Y() + room->Height() - 120));
		else
			teleporter->Position(Vector2(
				room->X() + room->Width() * 0.5f - (teleporter->Bounds().Right() - teleporter->Anchor().X),
				room->Y() + room->Height() - 60));
		m_teleporterList.push_back(teleporter);
		m_teleporterRoom.push_back((int) i);
		if (rl_trace())
			fprintf(stderr, "teleporter in %s room %d at (%.0f,%.0f)\n", n, (int) i,
				teleporter->X(), teleporter->Y());
	}

	/* The diary rooms (DiaryRoomObj, Bonus Tag 6): the template's
	 * "diary" object. X/Y is the sprite's anchor, W/H its scaled size,
	 * so the bounds are centred on it. */
	for (size_t i = 0; i < m_castle.rooms.size(); i++)
	{
		const Levels::PlacedRoom &r = m_castle.rooms[i];
		if (r.name != "Bonus" || r.tag != "6") continue;
		for (const Levels::ObjData &o : r.objs)
			if (o.name == "diary")
			{
				DiaryRoom d;
				d.room = (int) i;
				d.bounds = Rectangle((int) (o.x - o.width * 0.5f), (int) (o.y - o.height * 0.5f),
					(int) o.width, (int) o.height);
				d.x = o.x;
				d.y = o.y;
				m_diaries.push_back(d);
				if (rl_trace())
					fprintf(stderr, "diary in room %d at (%.0f,%.0f)\n", (int) i, o.x, o.y);
				break;
			}
	}
	if (SpriteSystem::SpriteLibrary::ContainsSprite("UpArrowSquare_Sprite"))
	{
		m_diaryArrow = new DS2DEngine::SpriteObj("UpArrowSquare_Sprite");
		m_diaryArrow->Flip(SpriteEffects::FlipHorizontally);   // DiaryRoomObj.Initialize
		m_diaryArrow->ForceDraw(true);
	}

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
	RefreshLocalTerrain();
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

	if (rl_trace())
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

	if (rl_trace())
		fprintf(stderr, "physics list: %d objects, player tag=%d\n",
		        m_physicsManager.NumObjects(), m_player->CollisionTypeTag());
	m_valid = true;
}

ProceduralLevelScreen::~ProceduralLevelScreen()
{
	/* The fonts own their sheet copy (2 MB of Junicode): without this
	 * every rebuilt castle leaks them. */
	m_goldFont.Free();
	m_objFont.Free();
	delete m_objPlate;
	for (TeleporterObj *teleporter : m_teleporterList)
		delete teleporter;
	delete m_diaryArrow;
}

bool ProceduralLevelScreen::TakeTeleporterRequest()
{
	bool requested = m_teleporterRequested;
	m_teleporterRequested = false;
	return requested;
}

bool ProceduralLevelScreen::DebugBonusSpawn(int tag, float &x, float &y)
{
	for (size_t i = 0; i < m_castle.rooms.size(); i++)
	{
		const Levels::PlacedRoom &r = m_castle.rooms[i];
		if (r.name == "Bonus" && atoi(r.tag.c_str()) == tag)
		{
			x = r.x + r.width * 0.5f;
			y = r.y + r.height - 300;   /* falls onto the room's floor */
			return true;
		}
	}
	return false;
}

bool ProceduralLevelScreen::TakeDiaryRequest(int &entryIndex)
{
	if (m_diaryRequest < 0)
		return false;
	entryIndex = m_diaryRequest;
	m_diaryRequest = -1;
	return true;
}

void ProceduralLevelScreen::TeleportToRoom(int roomIndex)
{
	/* MapObj.TeleportPlayer: the hero lands on the destination pad,
	 * standing on the room's floor like BossRoom::Teleport. */
	for (size_t i = 0; i < m_teleporterList.size(); i++)
	{
		if (m_teleporterRoom[i] != roomIndex || m_player == nullptr)
			continue;
		TeleporterObj *teleporter = m_teleporterList[i];
		Rectangle feet = m_player->TerrainBounds();
		m_player->X(teleporter->X());
		m_player->Y(teleporter->Y() - (feet.Bottom() - m_player->Y()) - 1);
		if (rl_trace())
			fprintf(stderr, "teleported to room %d at (%.0f,%.0f)\n", roomIndex, m_player->X(), m_player->Y());
		break;
	}
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
	else if (m_currentRoom != nullptr)
		for (size_t i = 0; i < m_currentRoom->EnemyList().size(); i++)   // frozen, but they still die
		{
			EnemyObj *enemy = m_currentRoom->EnemyList()[i];
			if (!enemy->IsKilled && !enemy->NonKillable && enemy->CurrentHealth <= 0)
				enemy->Kill(true);
		}
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

	/* The teleporter pads of the current room: the arrow while the hero
	 * stands on one (TeleporterObj.CollisionResponse), and Up opening
	 * the teleporter map (PlayerObj.cs line 1904). */
	for (TeleporterObj *teleporter : m_teleporterList)
	{
		if (!roomBounds.Intersects(teleporter->Bounds()))
			continue;
		if (m_player != nullptr && teleporter->Bounds().Intersects(playerBounds)
		    && m_player->IsTouchingGround() && !m_player->ControlsLocked)
		{
			teleporter->ArrowVisible = true;
			if (upPressed)
				m_teleporterRequested = true;
		}
	}

	/* The diary of the current room (DiaryRoomObj.Update): the bubble
	 * floats above it while the hero has not read it (or stands next to
	 * it), and Up reads the next journal entry. */
	m_elapsed += (float) gameTime->ElapsedGameTime;
	for (DiaryRoom &d : m_diaries)
	{
		if (m_roomList[d.room] != m_currentRoom || m_player == nullptr)
		{
			d.arrowVisible = false;
			continue;
		}
		/* DiaryRoomObj.cs line 63: the bounds widen by 50/100. */
		Rectangle b = d.bounds;
		b.X -= 50;
		b.Width += 100;
		bool touching = playerBounds.Intersects(b);
		bool maxed = m_diaryEntry != nullptr && *m_diaryEntry >= LevelEV::TOTAL_JOURNAL_ENTRIES - 1;
		d.arrowVisible = !maxed && (!d.used || touching);
		if (!touching || !m_player->IsTouchingGround() || !upPressed || m_diaryRequest >= 0)
			continue;
		/* DiaryRoomObj.cs line 71: the next entry is shown, the counter
		 * moves on and the room is completed for this castle. The
		 * journal list screen of the maxed diary is not ported. */
		if (!d.used && !maxed && m_diaryEntry != nullptr)
		{
			m_diaryRequest = *m_diaryEntry;
			(*m_diaryEntry)++;
			d.used = true;
		}
	}

	/* m_projectileManager.Update(gameTime): move the projectiles and
	 * destroy the dead or out-of-play ones. */
	for (size_t i = 0; i < m_projectiles.size(); )
	{
		ProjectileObj *proj = m_projectiles[i];
		if (m_timeStopped && proj->Source != nullptr && !proj->IsKilled)
		{
			i++;   /* the enemies' shots hang in the air during the time stop */
			continue;
		}
		proj->Update(gameTime);
		if (rl_hits())
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
		/* The current room and a margin for the doorways: the rooms
		 * around wait until the hero walks in */
		m_physicsManager.SetActiveBounds(Rectangle(b.X - 300, b.Y - 300,
		                                          b.Width + 2 * 300, b.Height + 2 * 300));
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

	if (rl_trace())
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

	if (rl_trace())
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
			RefreshLocalTerrain();
			for (EnemyObj *enemy : roomObj->EnemyList())
				enemy->ResetState();
			SpawnDementiaEnemy();
			m_player->RoomTransitionReset();
			/* RoomObj.OnEnter line 135 + DisplayFairyChestInfo (line
			 * 755): the objective plate follows the room's fairy
			 * chest, or leaves. */
			ResetObjectivePlate(false);
			for (ChestObj *chest : roomObj->ChestList())
			{
				if (!chest->IsFairyChest() || chest->IsOpen())
					continue;
				FairyChestObj *fairy = static_cast<FairyChestObj *>(chest);
				if (fairy->ConditionType() != ChestConditionType::None
				    && fairy->ConditionType() != ChestConditionType::InvisibleChest
				    && fairy->State == FairyChestObj::STATE_LOCKED)
				{
					char key[24];
					snprintf(key, sizeof(key), "Chest_Locked %d",
					         fairy->ConditionType());
					const char *desc = "";
					for (const Dialogue::Entry &e : Dialogue::ENTRIES)
						if (strcmp(e.name, key) == 0 && e.count > 0)
						{
							/* Speakers[0] carries the condition text */
							desc = Text::Loc::Get(e.pages[0].speaker);
							break;
						}
					DisplayObjective("LOC_ID_LEVEL_SCREEN_2", desc, "", true);
					break;
				}
			}
			if (getenv("RL_DEBUG_TRANSITIONS") != nullptr)
				fprintf(stderr, "room transition -> %s (%.0f, %.0f)\n",
				        roomObj->Name().c_str(), m_player->X(), m_player->Y());
			return;
		}
	}
}

/* The objective plate, in the 1320x720 virtual screen (the port's
 * play area): ProceduralLevelScreen.cs line 274-275, X 1170 shown and
 * 1470 hidden, Y 250, the box scaled to 250x130. */
static const float OBJ_PLATE_SHOWN = 1170;
static const float OBJ_PLATE_HIDDEN = 1470;
static float ObjSX(float v) { return (v - 660) * 0.5f + SCREEN_W / 2; }
static float ObjSY(float v) { return (v - 360) * 0.5f + 240; }

void ProceduralLevelScreen::Draw(Camera2D *camera)
{
	if (!m_valid)
		return;

	/* The original look (sky, background, textured walls, borders,
	 * decorations) of the rooms in view, then their objects on top.
	 * m_roomList is built in the order of m_castle.rooms. */
	Rectangle view = camera->Bounds();
	/* The sky once, before the rooms, when one of them lets it show */
	bool sky = false;
	for (size_t i = 0; i < m_castle.rooms.size() && i < m_roomList.size() && !sky; i++)
	{
		const Levels::PlacedRoom &r = m_castle.rooms[i];
		if (!Rectangle((int) r.x, (int) r.y, r.width, r.height).Intersects(view))
			continue;
		if (m_roomList[i] == m_currentRoom && m_roomRenderer.CacheIsFor(r) && !m_noRoomCache)
			sky = !m_roomRenderer.CacheOpaque();   /* the cache covers everything, or shows the holes */
		else
			sky = r.name == "Start" || m_roomRenderer.SkyShowsThrough(r.levelType);
	}
	uint64_t px0 = gfx_pixels;
	if (sky)
		m_roomRenderer.DrawSky(camera);
	else
		gfx_fill(0, 60, SCREEN_W, 360, 0xFF041D56u);   // the night blue the sky would start with
	uint64_t pxSky = gfx_pixels - px0;
	/* The current room from its cache (built the first frame in it: the
	 * short wait on entering a room), then the others live on top (their
	 * art may reach into it: a ramp of the next room) */
	size_t current = m_roomList.size();
	for (size_t i = 0; i < m_castle.rooms.size() && i < m_roomList.size(); i++)
		if (m_roomList[i] == m_currentRoom && !m_noRoomCache)
		{
			const Levels::PlacedRoom &r = m_castle.rooms[i];
			if (!m_roomRenderer.CacheIsFor(r))
				m_roomRenderer.BuildCache(camera, r, &m_roomList[i]->BGObjList());
			if (m_roomRenderer.CacheIsFor(r))
			{
				m_roomRenderer.DrawCached(camera, r, &m_roomList[i]->BGObjList());
				current = i;
			}
		}
	for (size_t i = 0; i < m_castle.rooms.size() && i < m_roomList.size(); i++)
	{
		const Levels::PlacedRoom &r = m_castle.rooms[i];
		if (i != current && Rectangle((int) r.x, (int) r.y, r.width, r.height).Intersects(view))
			m_roomRenderer.Draw(camera, r, &m_roomList[i]->BGObjList());
	}
	uint64_t pxRooms = gfx_pixels - px0 - pxSky;
	for (RoomObj *room : m_roomList)
		room->Draw(camera);
	for (TeleporterObj *teleporter : m_teleporterList)
		teleporter->Draw(camera);
	/* The diaries' bubble (DiaryRoomObj.cs line 65), swaying above. */
	if (m_diaryArrow != nullptr)
		for (DiaryRoom &d : m_diaries)
			if (d.arrowVisible)
			{
				float sway = sinf(m_elapsed * 20) * 2;
				m_diaryArrow->Position(Vector2(d.x, d.y - m_diaryArrow->SpriteRect().Height - 50 + sway));
				m_diaryArrow->Draw(camera);
			}
	if (rl_trace())
	{
		static int frames = 0;
		if (++frames % 120 == 0)
			fprintf(stderr, "overdraw: sky %lluk, room art %lluk, room objects %lluk\n", (unsigned long long) pxSky / 1000,
			        (unsigned long long) pxRooms / 1000, (unsigned long long) (gfx_pixels - px0 - pxSky - pxRooms) / 1000);
	}

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

	/* The fairy chest objective plate (ForceDraw in the C# = the
	 * 1320x720 virtual screen, the port's play area): the box at
	 * (1170, 250) scaled to 250x130, the texts as its children at
	 * (-400, -60), (+40) and (+35) more (line 251-273). */
	if (m_objX < OBJ_PLATE_HIDDEN - 0.5f && m_objPlate != nullptr)
	{
		uint32_t *frame = gfx_frame();
		uint32_t title = 0xFFFF4444u, text = 0xFFF1E4C7u;
		m_objPlate->Scale(Vector2(250.0f / 955, 130.0f / 180));
		m_objPlate->Position(Vector2(m_objX, 250));
		m_objPlate->Draw(&m_objCamera);
		m_objFont.DrawOutlined(frame, m_objTitle.c_str(), ObjSX(770), ObjSY(190),
		                       0.28f, title, 0xFF000000u, 1);
		m_objFont.DrawOutlined(frame, m_objDescription.c_str(), ObjSX(770), ObjSY(230),
		                       0.25f, text, 0xFF000000u, 1);
		if (!m_objProgress.empty())
			m_objFont.DrawOutlined(frame, m_objProgress.c_str(), ObjSX(770), ObjSY(265),
			                       0.25f, text, 0xFF000000u, 1);
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

void ProceduralLevelScreen::DisplayObjective(const char *titleID,
                                              const char *description,
                                              const char *progress, bool tween)
{
	/* ProceduralLevelScreen.cs line 2870 (DisplayObjective). */
	m_objTitle = Text::Loc::Get(titleID);
	m_objDescription = description != nullptr ? description : "";
	m_objProgress = progress != nullptr ? progress : "";
	m_objAlphaTitle = 1;
	m_objShown = true;
	m_objSlideFrom = m_objX;
	m_objSlideTo = OBJ_PLATE_SHOWN;
	m_objSlideStart = m_objTime;
	if (!tween)
		m_objX = OBJ_PLATE_SHOWN;
}

void ProceduralLevelScreen::ResetObjectivePlate(bool tween)
{
	/* ProceduralLevelScreen.cs line 2895: back out of the screen. */
	if (!m_objShown)
		return;
	m_objShown = false;
	m_objSlideFrom = m_objX;
	m_objSlideTo = OBJ_PLATE_HIDDEN;
	m_objSlideStart = m_objTime;
	if (!tween)
		m_objX = OBJ_PLATE_HIDDEN;
}

void ProceduralLevelScreen::UpdateObjectiveProgress(const std::string &progress)
{
	/* ProceduralLevelScreen.cs line 2912. */
	m_objProgress = progress;
}

void ProceduralLevelScreen::ObjectiveFailed()
{
	/* ProceduralLevelScreen.cs line 2917. */
	m_objTitle = Text::Loc::Get("LOC_ID_LEVEL_SCREEN_6");
	m_objAlphaTitle = 0.3f;
}

void ProceduralLevelScreen::ObjectiveComplete()
{
	/* ProceduralLevelScreen.cs line 2924. */
	m_objTitle = Text::Loc::Get("LOC_ID_LEVEL_SCREEN_5");
	m_objAlphaTitle = 0.3f;
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
	if (rl_trace())
		fprintf(stderr, "proj: %s (%.0f,%.0f) [add]\n",
		        proj->SpriteName().c_str(), proj->X(), proj->Y());
}

void ProceduralLevelScreen::RefreshLocalTerrain()
{
	m_allTerrain.clear();
	if (m_currentRoom == nullptr)
	{
		m_allTerrain = m_castleTerrain;
		return;
	}
	/* The room and the doorway margin of the physics' active bounds */
	Rectangle b = m_currentRoom->Bounds();
	Rectangle around(b.X - 300, b.Y - 300, b.Width + 600, b.Height + 600);
	for (DS2DEngine::IPhysicsObj *obj : m_castleTerrain)
	{
		Rectangle t = obj->TerrainBounds();
		if (t.X < around.X + around.Width && t.X + t.Width > around.X
		    && t.Y < around.Y + around.Height && t.Y + t.Height > around.Y)
			m_allTerrain.push_back(obj);
	}
}

void ProceduralLevelScreen::KillProjectilesFrom(EnemyObj *source)
{
	for (ProjectileObj *proj : m_projectiles)
		if (proj->Source == source)
			proj->KillProjectile();
}

ProjectileObj *ProceduralLevelScreen::FireProjectile(const char *spriteName, Vector2 position,
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
	if (rl_trace())
		fprintf(stderr, "proj: %s (%.0f,%.0f) h=(%.2f,%.2f) speed=%.0f\n",
		        spriteName, position.X, position.Y, heading.X, heading.Y, speed);
	return proj;
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
	if (rl_trace())
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
	/* by index, over the enemies there now: a splitting blob adds to the list */
	const std::vector<EnemyObj *> &list = m_currentRoom->EnemyList();
	size_t count = list.size();
	for (size_t i = 0; i < count && i < list.size(); i++)
	{
		EnemyObj *enemy = list[i];
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
	/* PauseAllEnemies + PauseAllProjectiles (line 2563) */
	if (m_currentRoom != nullptr)
		for (EnemyObj *enemy : m_currentRoom->EnemyList())
			enemy->PauseEnemy();
	for (ProjectileObj *proj : m_projectiles)
		if (proj->Source != nullptr)
		{
			proj->DisableAllWeightFlag = true;
			proj->PauseAnimation();
		}
}

void ProceduralLevelScreen::StopTimeStop()
{
	m_timeStopped = false;
	if (m_currentRoom != nullptr)
		for (EnemyObj *enemy : m_currentRoom->EnemyList())
			enemy->UnpauseEnemy();
	for (ProjectileObj *proj : m_projectiles)
		if (proj->Source != nullptr)
		{
			proj->DisableAllWeightFlag = false;
			proj->ResumeAnimation();
		}
}

} // namespace RogueCastle

namespace RogueCastle
{
EnemyObj *ProceduralLevelScreen::AddEnemyToCurrentRoom(uint8_t type, Vector2 position, const char *difficulty)
{
	if (m_currentRoom == nullptr) return nullptr;
	EnemyObj *enemy = m_currentRoom->SpawnTempEnemy(type, position, m_player, difficulty);
	if (enemy == nullptr) return nullptr;
	enemy->SetPlayerTarget(m_player);
	enemy->SetLevelScreen(this);
	enemy->SetTerrain(&m_allTerrain);
	m_physicsManager.AddObject(enemy);
	return enemy;
}

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
	EnemyObj *enemy = AddEnemyToCurrentRoom(type, model->Position());
	if (enemy == nullptr)
		return;
	enemy->Demented = true;
	enemy->NonKillable = true;
	if (rl_trace())
		fprintf(stderr, "dementia: enemy %d in %s\n", (int) type, room->Name().c_str());
}
}
