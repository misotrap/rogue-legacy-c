/* Port of RogueCastle/src/Game Objects/RoomObjs/RoomObj.cs (subset). */
#include "RoomObj.h"
#include "../../../debugflags.h"
#include <cmath>
#include <algorithm>
#include "../../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../../SpriteSystem/SpriteLibrary.h"
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
#include "../EnemyObjs/EnemyObj_ShieldKnight.h"
#include "../EnemyObjs/EnemyObj_FireWizard.h"
#include "../EnemyObjs/EnemyObj_IceWizard.h"
#include "../EnemyObjs/EnemyObj_EarthWizard.h"
#include "../EnemyObjs/EnemyObj_BallAndChain.h"
#include "../EnemyObjs/EnemyObj_SkeletonArcher.h"
#include "../EnemyObjs/EnemyObj_HomingTurret.h"
#include "../EnemyObjs/EnemyObj_Horse.h"
#include "../EnemyObjs/EnemyObj_Fireball.h"
#include "../EnemyObjs/EnemyObj_Mimic.h"
#include "../EnemyObjs/EnemyObj_Portrait.h"
#include "../EnemyObjs/EnemyObj_Chicken.h"
#include "../EnemyObjs/EnemyObj_Starburst.h"
#include "../EnemyObjs/EnemyObj_Energon.h"
#include "../EnemyObjs/EnemyObj_Fairy.h"
#include "../EnemyObjs/EnemyObj_LastBoss.h"
#include "../Player/PlayerObj.h"
#include "../ChestObj.h"
#include "../FairyChestObj.h"
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
	for (ChestObj *chest : m_chestList)
		delete chest;
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
	case EnemyType::ShieldKnight:
		return new EnemyObj_ShieldKnight(player, diff);
	case EnemyType::FireWizard:
		return new EnemyObj_FireWizard(player, diff);
	case EnemyType::IceWizard:
		return new EnemyObj_IceWizard(player, diff);
	case EnemyType::EarthWizard:
		return new EnemyObj_EarthWizard(player, diff);
	case EnemyType::BallAndChain:
		return new EnemyObj_BallAndChain(player, diff);
	case EnemyType::SkeletonArcher:
		return new EnemyObj_SkeletonArcher(player, diff);
	case EnemyType::HomingTurret:
		return new EnemyObj_HomingTurret(player, diff);
	case EnemyType::Horse:
		return new EnemyObj_Horse(player, diff);
	case EnemyType::Fireball:
		return new EnemyObj_Fireball(player, diff);
	case EnemyType::Mimic:
		return new EnemyObj_Mimic(player, diff);
	case EnemyType::Portrait:
		return new EnemyObj_Portrait(player, diff);
	case EnemyType::Chicken:
		return new EnemyObj_Chicken(player, diff);
	case EnemyType::Starburst:
		return new EnemyObj_Starburst(player, diff);
	case EnemyType::Energon:
		return new EnemyObj_Energon(player, diff);
	case EnemyType::Fairy:
		return new EnemyObj_Fairy(player, diff);
	case EnemyType::LastBoss:
		return new EnemyObj_LastBoss(player, diff);
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
			{
				if (obj.rotation == 45 && obj.collidesTop)
					m_slopeList.push_back({ obj.x, obj.y, obj.width, obj.height });
				continue;
			}
			TerrainObj *terrain = new TerrainObj((int) obj.width, (int) obj.height);
			terrain->Populate((int) obj.x, (int) obj.y, (int) obj.width, (int) obj.height,
			                  obj.collidesTop, obj.collidesBottom, obj.collidesLeft, obj.collidesRight);
			m_terrainList.push_back(terrain);
			if (obj.name.compare(0, 9, "Coll_Hull") != 0)
				m_namedTerrain.emplace_back(obj.name, terrain);
		}
		/* HazardObj (HazardObj.cs): the spikes, turned around their top-left
		 * corner. Spikes_Sprite's boxes: the solid part (10, 15, width-25,
		 * 40) and the one that hurts (0, 0, width, 60) */
		else if (obj.type == "HazardObj")
		{
			auto turned = [&](float ox, float oy, float w, float h)
			{
				float rad = obj.rotation * 3.14159265f / 180.0f, c = cosf(rad), s = sinf(rad);
				float xs[4] = { ox, ox + w, ox + w, ox }, ys[4] = { oy, oy, oy + h, oy + h };
				float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f;
				for (int i = 0; i < 4; i++)
				{
					float px = obj.x + xs[i] * c - ys[i] * s, py = obj.y + xs[i] * s + ys[i] * c;
					x0 = fminf(x0, px); x1 = fmaxf(x1, px); y0 = fminf(y0, py); y1 = fmaxf(y1, py);
				}
				return Hazard { roundf(x0), roundf(y0), roundf(x1 - x0), roundf(y1 - y0) };
			};
			Hazard solid = turned(10, 15, obj.width - 25, 40);
			TerrainObj *terrain = new TerrainObj((int) solid.width, (int) solid.height);
			terrain->Populate((int) solid.x, (int) solid.y, (int) solid.width, (int) solid.height, true, true, true, true);
			m_terrainList.push_back(terrain);
			m_hazardList.push_back(turned(0, 0, obj.width, 60));
		}
		/* Collidable PhysicsObj (bookcases...): WALL_FOR_PLAYER, solid on
		 * top only (LevelParser.cs line 161), so the hero can stand on them */
		else if (obj.type == "PhysicsObj" && obj.collidable && obj.rotation == 0
		         && SpriteSystem::SpriteLibrary::ContainsSprite(obj.sprite))
		{
			DS2DEngine::SpriteObj sprite(obj.sprite);
			sprite.Position(Vector2(obj.x, obj.y));
			sprite.Scale(Vector2(obj.scaleX, obj.scaleY));
			sprite.Flip(obj.flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);
			Rectangle b = sprite.Bounds();
			TerrainObj *terrain = new TerrainObj(b.Width, b.Height);
			terrain->Populate(b.X, b.Y, b.Width, b.Height, true, false, false, false);
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
		{
			BreakableObj *b = new BreakableObj(obj.sprite);
			b->Name = obj.name;   /* "Health"/"Mana" force a drop */
			container = b;
			m_breakableList.push_back(b);
		}
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
		container->IsCollidableFlag = breakable;   /* LevelParser: breakables always collide */
		container->IsWeightedFlag = breakable && obj.weighted;
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
		/* BGLayer (Layer -1): behind the walls (RoomObj.DrawBGObjs) */
		if (obj.bgLayer && !breakable)
			m_bgDrawableList.push_back(container);
		else
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
		enemy->Flip(obj.flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);   /* LevelParser.cs line 115 */
		enemy->Tag = obj.tag;
		enemy->Procedural = obj.procedural;
		enemy->Initialize();
		enemy->InitialLogicDelay(obj.initialDelay);
		m_enemyList.push_back(enemy);
		m_drawableList.push_back(enemy);
	}

	/* Chests (LevelParser.cs line 84: Fairy=true makes a FairyChestObj).
	 * The chest type roll and the recentring happen in the level's
	 * InitializeChests. */
	for (const Levels::ObjData &obj : placed.objs)
	{
		if (obj.type != "ChestObj")
			continue;
		ChestObj *chest = obj.fairy ? (ChestObj *) new FairyChestObj() : new ChestObj();
		chest->X(obj.x);
		chest->Y(obj.y);
		chest->Flip(obj.flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);
		if (obj.fairy)
			((FairyChestObj *) chest)->Tag = obj.tag;
		m_chestList.push_back(chest);
		m_drawableList.push_back(chest);
	}
}

void RoomObj::Update(GameTime *gameTime)
{
	/* CurrentRoom.Update(gameTime): the enemies of the active room. */
	/* By index: a dying enemy may add others (the ball knight's spikes) */
	for (size_t i = 0; i < m_enemyList.size(); i++)
	{
		EnemyObj *enemy = m_enemyList[i];
		enemy->Update(gameTime);
		if (rl_trace())
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

EnemyObj *RoomObj::SpawnTempEnemy(uint8_t type, Vector2 position, PlayerObj *player, const char *difficulty)
{
	Levels::ObjData data;
	data.enemyType = type;
	data.difficulty = difficulty;
	EnemyObj *enemy = CreateEnemy(data, player);
	if (enemy == nullptr)
		return nullptr;
	enemy->X(position.X);
	enemy->Y(position.Y);
	enemy->Initialize();
	m_tempEnemies.push_back(enemy);
	m_enemyList.push_back(enemy);
	m_drawableList.push_back(enemy);
	return enemy;
}

void RoomObj::ClearTempEnemies(DS2DEngine::PhysicsManager &physics)
{
	for (EnemyObj *enemy : m_tempEnemies)
	{
		physics.RemoveObject(enemy);
		m_enemyList.erase(std::remove(m_enemyList.begin(), m_enemyList.end(), enemy), m_enemyList.end());
		m_drawableList.erase(std::remove(m_drawableList.begin(), m_drawableList.end(), enemy), m_drawableList.end());
		delete enemy;
	}
	m_tempEnemies.clear();
}

TerrainObj *RoomObj::TerrainNamed(const std::string &name) const
{
	for (const auto &t : m_namedTerrain)
		if (t.first == name) return t.second;
	return nullptr;
}

} // namespace RogueCastle
