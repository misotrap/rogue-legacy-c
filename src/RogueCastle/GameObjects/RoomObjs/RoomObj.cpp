/* Port of RogueCastle/src/Game Objects/RoomObjs/RoomObj.cs (subset). */
#include "RoomObj.h"
#include "../../../gfx.h"

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
}

void RoomObj::LoadRoom(const Levels::PlacedRoom &placed)
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

void RoomObj::Draw(Camera2D *camera)
{
	if (!Bounds().Intersects(camera->Bounds()))
		return;

	/* Walls: plain coloured rectangles (AGENTS.md), via the camera
	 * transform. */
	for (const TerrainObj *terrain : m_terrainList)
	{
		Rectangle b = terrain->TerrainBounds();
		camera->DrawRectangle(Vector2((float) b.X, (float) b.Y), b.Width, b.Height,
		                       terrain->CollidesBottom() || terrain->CollidesLeft() || terrain->CollidesRight()
		                           ? WallColour() : DoorColour());
	}

	/* Door openings: a brighter strip so the passages read visually. */
	for (const DoorInfo &door : m_doorList)
		camera->DrawRectangle(Vector2(door.x, door.y), door.width, door.height, DoorColour());
}

} // namespace RogueCastle
