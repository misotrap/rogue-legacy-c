/* Port of RogueCastle/src/Game Objects/RoomObjs/RoomObj.cs (subset): one
 * placed room of the castle at runtime, with its terrain, doors and area
 * colours. Enemies, breakables and the other objects join with their
 * milestones.
 */
#ifndef ROGUE_CASTLE_ROOM_OBJ_H
#define ROGUE_CASTLE_ROOM_OBJ_H

#include "../../../DS2DEngine/Camera2D.h"
#include "../../Levels/RoomData.h"
#include "../TerrainObj.h"
#include "../BreakableObj.h"
#include <string>
#include <vector>

namespace RogueCastle
{

class RoomObj
{
public:
	RoomObj();
	~RoomObj();

	/* Builds the room from the LevelBuilder2 output (castle coordinates).
	 * The player is passed as the enemies' target. */
	void LoadRoom(const Levels::PlacedRoom &placed, class PlayerObj *player);
	void Update(GameTime *gameTime);
	/* Debug fallback: plain-colour walls instead of RoomRenderer */
	static bool PlainWalls;
	void Draw(DS2DEngine::Camera2D *camera);

	const std::string &Name() const { return m_name; }
	const std::string &Tag() const { return m_tag; }
	Levels::LevelType LevelType() const { return m_levelType; }
	float X() const { return m_x; }
	float Y() const { return m_y; }
	int Width() const { return m_width; }
	int Height() const { return m_height; }
	int LinkedRoom() const { return m_linkedRoom; }

	Rectangle Bounds() const
	{
		return Rectangle((int) m_x, (int) m_y, m_width, m_height);
	}
	bool Contains(int x, int y) const
	{
		return x >= (int) m_x && x < (int) m_x + m_width &&
		       y >= (int) m_y && y < (int) m_y + m_height;
	}

	const std::vector<TerrainObj *> &TerrainObjList() const { return m_terrainList; }
	/* Breakables and decoration containers, drawn and simulated by the room. */
	const std::vector<DS2DEngine::PhysicsObjContainer *> &GameObjList() const { return m_gameObjList; }
	const std::vector<class BreakableObj *> &BreakableObjList() const { return m_breakableList; }
	const std::vector<class ChestObj *> &ChestList() const { return m_chestList; }
	const std::vector<class EnemyObj *> &EnemyList() const { return m_enemyList; }
	int ActiveEnemies() const;
	/* RoomObj.Level: the room counter used by the chest level and the
	 * enemy difficulty (ProceduralLevelScreen.cs line 886). */
	int Level = 0;
	const std::vector<DS2DEngine::GameObj *> &DrawableObjList() const { return m_drawableList; }

	/* Colours: one background and wall tone per area (AGENTS.md). */
	uint32_t BackgroundColour() const;
	uint32_t WallColour() const;
	uint32_t DoorColour() const;


private:
	std::string m_name;
	std::string m_tag;
	Levels::LevelType m_levelType = Levels::LevelType::CASTLE;
	float m_x = 0, m_y = 0;
	int m_width = 0, m_height = 0;
	int m_linkedRoom = -1;
	std::vector<TerrainObj *> m_terrainList;
	std::vector<DS2DEngine::PhysicsObjContainer *> m_gameObjList;   /* owned */
	std::vector<class BreakableObj *> m_breakableList;              /* = the breakables of m_gameObjList */
	std::vector<class ChestObj *> m_chestList;                      /* owned */
	std::vector<class EnemyObj *> m_enemyList;                     /* owned */
	std::vector<DS2DEngine::GameObj *> m_drawableList;               /* = m_gameObjList */

	struct DoorInfo
	{
		Levels::DoorPos pos = Levels::DoorPos::None;
		float x = 0, y = 0;
		int width = 0, height = 0;
		bool bossDoor = false;
	};
	std::vector<DoorInfo> m_doorList;
};

} // namespace RogueCastle

#endif
