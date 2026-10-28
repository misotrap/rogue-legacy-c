/* Port of RogueCastle/src/Screens/ProceduralLevelScreen.cs (subset). */
#include "ProceduralLevelScreen.h"
#include "../EVs/GlobalEV.h"
#include "../Game.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/ProjectileObj.h"
#include "../../DS2DEngine/CollisionMath.h"
#include "../../gfx.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

ProceduralLevelScreen::ProceduralLevelScreen(const char *assets, uint32_t seed)
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
	SpriteSystem::SpriteLibrary::LoadSheet(assets, "enemyFinal2Spritesheet");
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
	m_physicsManager.AddObject(m_player);

	for (const Levels::PlacedRoom &placed : m_castle.rooms)
	{
		RoomObj *room = new RoomObj();
		room->LoadRoom(placed, m_player);
		m_roomList.push_back(room);
	}

	/* Every room's terrain lives in one world, so the player can walk
	 * between rooms through the door openings. */
	for (RoomObj *room : m_roomList)
	{
		for (TerrainObj *terrain : room->TerrainObjList())
		{
			m_physicsManager.AddObject(terrain);
			m_allTerrain.push_back(terrain);
		}
		for (DS2DEngine::PhysicsObjContainer *obj : room->GameObjList())
			m_physicsManager.AddObject(obj);
		for (EnemyObj *enemy : room->EnemyList())
		{
			enemy->SetPlayerTarget(m_player);
			enemy->SetLevelScreen(this);
			enemy->SetTerrain(&m_allTerrain);
			m_physicsManager.AddObject(enemy);
		}
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

	/* ProceduralLevelScreen.cs line 1536: room, player, then physics. */
	if (m_currentRoom != nullptr)
		m_currentRoom->Update(gameTime);
	if (m_player != nullptr)
		m_player->Update(gameTime);

	/* m_projectileManager.Update(gameTime): move the projectiles and
	 * destroy the dead or out-of-play ones. */
	for (size_t i = 0; i < m_projectiles.size(); )
	{
		ProjectileObj *proj = m_projectiles[i];
		proj->Update(gameTime);
		Rectangle level = m_currentRoom != nullptr
		    ? m_currentRoom->Bounds()
		    : Rectangle(-100000, -100000, 200000, 200000);
		bool outOfPlay = proj->X() < level.X - 600 || proj->X() > level.Right() + 600
		              || proj->Y() > level.Bottom() + 600 || proj->Y() < level.Y - 1200;
		if (proj->IsKilled || (!proj->Held && outOfPlay))
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
				for (EnemyObj *enemy : m_currentRoom->EnemyList())
					enemy->ResetState();
			m_currentRoom = roomObj;
			for (EnemyObj *enemy : roomObj->EnemyList())
				enemy->ResetState();
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
			m_roomRenderer.Draw(camera, r);
	}
	for (RoomObj *room : m_roomList)
		room->Draw(camera);

	for (ProjectileObj *proj : m_projectiles)
		proj->Draw(camera);
	if (m_player != nullptr)
		m_player->Draw(camera);
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
                                           float lifeSpan)
{
	ProjectileObj *proj = new ProjectileObj(spriteName, position, heading, speed,
	                                        damage, scale, weighted);
	proj->Source = source;
	proj->LifeSpan = lifeSpan;
	m_projectiles.push_back(proj);
	m_physicsManager.AddObject(proj);
	if (getenv("RL_DEBUG_TRACE") != nullptr)
		fprintf(stderr, "proj: %s (%.0f,%.0f) h=(%.2f,%.2f) speed=%.0f\n",
		        spriteName, position.X, position.Y, heading.X, heading.Y, speed);
}

} // namespace RogueCastle
