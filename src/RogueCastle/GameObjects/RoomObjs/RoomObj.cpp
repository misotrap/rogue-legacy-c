/* Port of RogueCastle/src/Game Objects/RoomObjs/RoomObj.cs (subset). */
#include "RoomObj.h"
#include "../EnemyObjs/EnemyObj.h"
#include "../EnemyObjs/EnemyObj_Skeleton.h"
#include "../EnemyObjs/EnemyObj_Knight.h"
#include "../EnemyObjs/EnemyObj_Eyeball.h"
#include "../EnemyObjs/EnemyObj_Blob.h"
#include "../EnemyObjs/EnemyObj_Turret.h"
#include "../EnemyObjs/EnemyObj_SpikeTrap.h"
#include "../EnemyObjs/EnemyObj_BouncySpike.h"
#include "../EnemyObjs/EnemyObj_SwordKnight.h"
#include "../EnemyObjs/EnemyObj_Plant.h"
#include "../EnemyObjs/EnemyObj_Spark.h"
#include "../EnemyObjs/EnemyObj_Zombie.h"
#include "../EnemyObjs/EnemyObj_Wolf.h"
#include "../EnemyObjs/EnemyObj_Ninja.h"
#include "../Player/PlayerObj.h"
#include "../../Types/EnemyType.h"
#include <cstdlib>
#include "../../../gfx.h"
#include "../../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

RoomObj::RoomObj()
{
}

RoomObj::~RoomObj()
{
	for (TerrainObj *terrain : m_terrainList)
		delete terrain;
	for (DS2DEngine::PhysicsObjContainer *obj : m_gameObjList)
		delete obj;
	for (EnemyObj *enemy : m_enemyList)
		delete enemy;
}

/* Creates an enemy from its EnemyObj room data (the C# spawns every
 * template enemy when the room loads); returns nullptr for the enemy
 * types that are not ported yet. */
static EnemyObj *CreateEnemy(const Levels::ObjData &obj, PlayerObj *player)
{
	using Diff = EnemyObj::EnemyDifficulty;
	Diff diff = Diff::BASIC;
	if (obj.difficulty == "ADVANCED")
		diff = Diff::ADVANCED;
	else if (obj.difficulty == "EXPERT")
		diff = Diff::EXPERT;
	else if (obj.difficulty == "MINIBOSS")
		diff = Diff::MINIBOSS;

	switch (obj.enemyType)
	{
	case EnemyType::Skeleton:
		return new EnemyObj_Skeleton(player, diff);
	case EnemyType::Knight:
		return new EnemyObj_Knight(player, diff);
	case EnemyType::Eyeball:
		return new EnemyObj_Eyeball(player, diff);
	case EnemyType::Blob:
		return new EnemyObj_Blob(player, diff);
	case EnemyType::Turret:
		return new EnemyObj_Turret(player, diff);
	case EnemyType::SpikeTrap:
		return new EnemyObj_SpikeTrap(player, diff);
	case EnemyType::BouncySpike:
		return new EnemyObj_BouncySpike(player, diff);
	case EnemyType::SwordKnight:
		return new EnemyObj_SwordKnight(player, diff);
	case EnemyType::Plant:
		return new EnemyObj_Plant(player, diff);
	case EnemyType::Spark:
		return new EnemyObj_Spark(player, diff);
	case EnemyType::Zombie:
		return new EnemyObj_Zombie(player, diff);
	case EnemyType::Wolf:
		return new EnemyObj_Wolf(player, diff);
	case EnemyType::Ninja:
		return new EnemyObj_Ninja(player, diff);
	default:
		return nullptr;
	}
}

void RoomObj::LoadRoom(const Levels::PlacedRoom &placed, PlayerObj *player)
{
	m_name = placed.name;
	m_tag = placed.tag;
	m_levelType = placed.levelType;
	m_x = placed.x;
	m_y = placed.y;
	m_width = placed.width;
	m_height = placed.height;
	m_linkedRoom = placed.linkedRoom;

	/* Terrain from the CollHullObj (and builder-added terrain like closed
	 * doors and door borders). Rotated hulls are rare; they are skipped
	 * until the blitter/collision supports rotation. */
	for (const Levels::ObjData &obj : placed.objs)
	{
		if (obj.type == "CollHullObj")
		{
			if (obj.rotation != 0)
				continue;
			TerrainObj *terrain = new TerrainObj((int) obj.width, (int) obj.height);
			terrain->Populate((int) obj.x, (int) obj.y, (int) obj.width, (int) obj.height,
			                  obj.collidesTop, obj.collidesBottom, obj.collidesLeft, obj.collidesRight);
			m_terrainList.push_back(terrain);
		}
	}

	for (const Levels::Door &door : placed.doors)
	{
		DoorInfo info;
		info.pos = door.pos;
		info.x = door.x;
		info.y = door.y;
		info.width = door.width;
		info.height = door.height;
		info.bossDoor = door.bossDoor;
		m_doorList.push_back(info);
	}

	/* Breakables (PhysicsObjContainer, Breakable=True) and decoration
	 * containers (the asset-swap objects of the area). */
	for (const Levels::ObjData &obj : placed.objs)
	{
		if ((obj.type != "ObjContainer" && obj.type != "PhysicsObjContainer") || obj.sprite.empty())
			continue;
		bool breakable = obj.breakable || obj.type == "PhysicsObjContainer";
		DS2DEngine::PhysicsObjContainer *container = nullptr;
		if (breakable)
			container = new BreakableObj(obj.sprite);
		else
			container = new DS2DEngine::PhysicsObjContainer(obj.sprite);
		container->X(obj.x);
		container->Y(obj.y);
		container->Scale(Vector2(obj.scaleX, obj.scaleY));
		container->Flip(obj.flip ? SpriteEffects::FlipHorizontally
		                         : SpriteEffects::None);
		if (obj.rotation != 0)
		{
			/* Rotated containers need blitter rotation support. */
			delete container;
			continue;
		}
		container->IsCollidableFlag = breakable;
		container->IsWeightedFlag = breakable;
		if (breakable)
		{
			container->UpdateCollisionBoxes();
			/* Solidity: a terrain box over the container bounds
			 * (ProceduralLevelScreen.cs line 951); RelativeBounds() are
			 * relative to its origin, like the box coordinates. */
			Rectangle b = container->RelativeBounds();
			container->AddCollisionBox(b.X, b.Y, b.Width, b.Height, Consts::TERRAIN_HITBOX);
			/* ProceduralLevelScreen.cs line 955: the breakable's own boxes
			 * are static, otherwise the per-frame UpdateCollisionBoxes
			 * overwrites the terrain bounds with the children union. */
			container->DisableHitboxUpdatingFlag = true;
			container->UpdateTerrainBox();
		}
		else
		{
			container->StopAnimation();
			container->PlayAnimation(true);
		}
		m_gameObjList.push_back(container);
		m_drawableList.push_back(container);
	}

	/* The room's enemies (LevelBuilder2 resolved the procedural ones into
	 * a type + difficulty). */
	for (const Levels::ObjData &obj : placed.objs)
	{
		if (obj.type != "EnemyObj")
			continue;
		EnemyObj *enemy = CreateEnemy(obj, player);
		if (enemy == nullptr)
			continue;
		enemy->X(obj.x);
		enemy->Y(obj.y);
		enemy->Rotation(obj.rotation);   /* the turret's firing angle */
		enemy->Tag = obj.tag;
		enemy->Initialize();
		enemy->InitialLogicDelay(obj.initialDelay);
		m_enemyList.push_back(enemy);
		m_drawableList.push_back(enemy);
	}
}

void RoomObj::Update(GameTime *gameTime)
{
	/* CurrentRoom.Update(gameTime): the enemies of the active room. */
	for (EnemyObj *enemy : m_enemyList)
	{
		enemy->Update(gameTime);
		if (getenv("RL_DEBUG_TRACE") != nullptr)
			fprintf(stderr, "e: %s %s vis=%d killed=%d x=%.0f y=%.0f hp=%.0f\n",
			        Name().c_str(), enemy->SpriteName().c_str(), (int) enemy->Visible(),
			        (int) enemy->IsKilled, enemy->X(), enemy->Y(), enemy->CurrentHealth);
	}
}

int RoomObj::ActiveEnemies() const
{
	int n = 0;
	for (const EnemyObj *enemy : m_enemyList)
		if (!enemy->IsKilled)
			n++;
	return n;
}

uint32_t RoomObj::BackgroundColour() const
{
	switch (m_levelType)
	{
	case Levels::LevelType::GARDEN: return 0xFF1C2418u;
	case Levels::LevelType::TOWER: return 0xFF1C2434u;
	case Levels::LevelType::DUNGEON: return 0xFF241C1Cu;
	default: return 0xFF241C28u;   /* castle */
	}
}

uint32_t RoomObj::WallColour() const
{
	switch (m_levelType)
	{
	case Levels::LevelType::GARDEN: return 0xFF33422Cu;
	case Levels::LevelType::TOWER: return 0xFF323E56u;
	case Levels::LevelType::DUNGEON: return 0xFF443034u;
	default: return 0xFF3A3244u;   /* castle */
	}
}

uint32_t RoomObj::DoorColour() const
{
	switch (m_levelType)
	{
	case Levels::LevelType::GARDEN: return 0xFF44583Cu;
	case Levels::LevelType::TOWER: return 0xFF42526Eu;
	case Levels::LevelType::DUNGEON: return 0xFF583E44u;
	default: return 0xFF4A4058u;   /* castle */
	}
}

bool RoomObj::PlainWalls = false;

void RoomObj::Draw(Camera2D *camera)
{
	if (!Bounds().Intersects(camera->Bounds()))
		return;

	/* Walls and doorways are drawn by RoomRenderer (original look); the
	 * plain rectangles below are only a fallback when PlainWalls is set. */
	if (PlainWalls)
	for (const TerrainObj *terrain : m_terrainList)
	{
		Rectangle b = terrain->TerrainBounds();
		camera->DrawRectangle(Vector2((float) b.X, (float) b.Y), b.Width, b.Height,
		                       terrain->CollidesBottom() || terrain->CollidesLeft() || terrain->CollidesRight()
		                           ? WallColour() : DoorColour());
	}

	/* Door openings: a brighter strip so the passages read visually. */
	if (PlainWalls)
	for (const DoorInfo &door : m_doorList)
		camera->DrawRectangle(Vector2(door.x, door.y), door.width, door.height, DoorColour());

	/* Breakables and decorations. */
	for (DS2DEngine::GameObj *obj : m_drawableList)
		obj->Draw(camera);
}

} // namespace RogueCastle
