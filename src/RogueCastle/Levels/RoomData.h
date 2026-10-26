// Room templates and the castle built from them.
//
// The original keeps eight full object copies of every room (normal and
// mirrored, for each of the four areas: LevelParser.cs, LevelBuilder2.cs
// StoreRoom). Here a template exists once; the area and the mirroring are
// applied when a room is placed in the castle (LevelBuilder2::Materialize).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Levels
{
	// GameTypes.LevelType (Types/GameTypes.cs)
	enum class LevelType : uint8_t { NONE, CASTLE, GARDEN, DUNGEON, TOWER };

	// DoorObj.DoorPosition ("Left", "Right", "Top", "Bottom", "None")
	enum class DoorPos : uint8_t { None, Left, Right, Top, Bottom };

	DoorPos Opposite(DoorPos p);
	const char *DoorPosName(DoorPos p);

	// One <GameObject> of a room, as written by tools/convert_assets.py.
	// Coordinates are relative to the room's top-left corner.
	struct ObjData
	{
		std::string type;      // CollHullObj, DoorObj, BorderObj, EnemyObj...
		std::string name;      // also carries the Top/!Top... door rules
		std::string sprite;    // SpriteName, when there is one
		std::string tag;
		float x = 0, y = 0;
		float width = 0, height = 0;
		float scaleX = 1, scaleY = 1;
		float rotation = 0;
		bool flip = false;
		int levelType = 0;     // LevelType of the original object (0 = all areas)
		bool bgLayer = false;  // BGLayer: drawn behind (Layer -1)

		// DoorObj
		DoorPos doorPos = DoorPos::None;
		bool bossDoor = false;
		// CollHullObj / BorderObj / PhysicsObj sides
		bool collidesTop = true, collidesBottom = true, collidesLeft = true, collidesRight = true;
		// EnemyObj
		bool procedural = false;
		int enemyType = 0;
		std::string difficulty; // BASIC, ADVANCED, EXPERT, MINIBOSS
		float initialDelay = 0;
		// EnemyOrbObj
		int orbType = 0;
		bool forceFlying = false, isWaypoint = false;
		// ChestObj / PhysicsObjContainer
		bool fairy = false, breakable = false;
	};

	struct RoomTemplate
	{
		std::string name;      // "Start", "Linker", "Boss", "Secret", "Bonus"...
		std::string tag;       // boss / bonus room type for special rooms
		std::string mapFile;   // Map_1x1.xml...
		float x = 0, y = 0;    // position in the editor map (rooms with x >= 10000 are unused)
		int width = 0, height = 0;
		bool castlePool = false, gardenPool = false, towerPool = false, dungeonPool = false;
		bool isDLCMap = false;
		std::vector<ObjData> objs;

		bool InPool(LevelType t) const;
	};

	// A door of a placed room, in castle coordinates
	struct Door
	{
		DoorPos pos = DoorPos::None;
		float x = 0, y = 0;
		int width = 0, height = 0;
		bool bossDoor = false;
		bool attached = false;
		bool locked = false;
	};

	// A room of the generated castle. Objects are only filled in at the end
	// (Materialize), so generating and retrying layouts stays cheap.
	struct PlacedRoom
	{
		int templ = -1;            // index into the template list
		bool reversed = false;     // mirrored left/right (RoomObj.Reverse)
		LevelType levelType = LevelType::NONE;
		float x = 0, y = 0;        // castle position of the top-left corner
		int width = 0, height = 0;
		std::string name, tag;
		std::vector<Door> doors;
		int linkedRoom = -1;       // boss entrance <-> boss room, castle entrance -> tutorial
		// Terrain and borders added by the builder (closed doors, door
		// borders, bottom platforms), relative to the room
		std::vector<ObjData> extra;
		std::vector<ObjData> objs; // castle coordinates, after Materialize

		float Right() const { return x + width; }
		float Bottom() const { return y + height; }
	};

	struct Castle
	{
		std::vector<PlacedRoom> rooms; // rooms[0] is the starting room
	};
}
