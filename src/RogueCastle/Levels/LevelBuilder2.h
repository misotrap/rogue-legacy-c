// Castle generation: port of LevelBuilder2.cs (CreateLevel and helpers).
//
// Usage:
//   std::vector<Levels::RoomTemplate> rooms;
//   Levels::LoadRooms("assets/rooms.txt", rooms);
//   Levels::LevelBuilder2 builder(rooms);
//   Levels::Castle castle;
//   builder.CreateLevel(castle, Levels::DefaultAreas(), 4, seed);
//
// The result is a list of placed rooms with their doors, linked boss rooms
// and every object in castle coordinates (procedural enemies resolved).
// Behaviour of special rooms (bonus, boss, challenge rooms) is not here:
// they keep their template name and tag ("Bonus" + tag = BonusRoomTypes).
#pragma once

#include "RoomData.h"

#include <random>

namespace Levels
{
	// AreaStruct (LevelBuilder2.cs), values from Game.cs
	struct AreaInfo
	{
		LevelType levelType = LevelType::CASTLE;
		int totalRoomsMin = 0, totalRoomsMax = 0;
		int secretRoomsMin = 0, secretRoomsMax = 0;
		int bonusRoomsMin = 0, bonusRoomsMax = 0;
		bool bossInArea = true;
	};

	// The four areas of a normal game (Game.cs CastleZone, GardenZone,
	// TowerZone, DungeonZone)
	const AreaInfo *DefaultAreas();

	class LevelBuilder2
	{
	public:
		explicit LevelBuilder2(const std::vector<RoomTemplate> &templates);

		// Builds a whole castle. Returns false if the room data lacks a
		// required special room (start, castle entrance, linkers...).
		bool CreateLevel(Castle &castle, const AreaInfo *areas, int numAreas, uint32_t seed);

		// Set when the diary is complete: diary bonus rooms are skipped
		bool diaryComplete = false;

	private:
		struct Candidate
		{
			int templ;
			bool reversed;
			bool operator==(const Candidate &o) const { return templ == o.templ && reversed == o.reversed; }
		};
		struct DoorRef
		{
			int room;   // index into the working room list
			int door;   // index into that room's doors
		};
		struct SuitableDoor
		{
			Candidate cand;
			int door;   // index into the template's doors (for that orientation)
		};

		const std::vector<RoomTemplate> &T;
		std::mt19937 rng;

		// Pools (LevelBuilder2.StoreRoom / StoreSpecialRoom)
		std::vector<Candidate> areaPool[5][4][4];
		std::vector<Candidate> secretPool[5], bonusPool[5], dlcPool[5];
		std::vector<int> bossRooms, challengeRooms;
		int startRoom = -1, castleEntrance = -1, tutorialRoom = -1, throneRoom = -1, endingRoom = -1, compassRoom = -1;
		int linker[5] = { -1, -1, -1, -1, -1 };
		int bossEntrance[5] = { -1, -1, -1, -1, -1 };
		// Doors of each template, normal [0] and mirrored [1], room-relative
		std::vector<std::vector<Door>> templDoors[2];

		void Index();
		const std::vector<Door> &DoorsOf(const Candidate &c) const { return templDoors[c.reversed ? 1 : 0][c.templ]; }
		int RandomInt(int min, int max); // CDGMath.RandomInt: inclusive
		PlacedRoom Instantiate(const Candidate &c, LevelType type, float x, float y) const;
		static void MoveRoom(PlacedRoom &room, float x, float y);

		bool CreateArea(int areaSize, const AreaInfo &area, std::vector<PlacedRoom> &rooms,
			int startingRoom, bool firstRoom, std::vector<int> &areaRooms);
		void FindSuitableDoors(const Door &needsLinking, const PlacedRoom &needsRoom, const std::vector<PlacedRoom> &rooms,
			LevelType type, bool findRoomsWithMoreDoors, std::vector<SuitableDoor> &out) const;
		void FindSuitableDoors(const Door &needsLinking, const PlacedRoom &needsRoom, const std::vector<Candidate> &pool,
			const std::vector<PlacedRoom> &rooms, std::vector<SuitableDoor> &out) const;
		bool CheckForRoomCollision(const Door &needsLinking, const PlacedRoom &needsRoom,
			const std::vector<PlacedRoom> &rooms, const Candidate &c, int door) const;
		bool FindFurthestDoor(const std::vector<PlacedRoom> &rooms, const std::vector<int> &subset, DoorPos direction,
			DoorPos doorWanted, bool castleOnly, DoorRef &found) const;
		bool AddLinkerToRoom(std::vector<PlacedRoom> &rooms, DoorRef needsLinking, DoorPos doorWanted, DoorRef &result);

		static void CloseRemainingDoors(std::vector<PlacedRoom> &rooms);
		static void AddDoorBorders(std::vector<PlacedRoom> &rooms);
		static void AddBottomPlatforms(std::vector<PlacedRoom> &rooms);
		void LinkAllBossEntrances(std::vector<PlacedRoom> &rooms);
		void Materialize(PlacedRoom &room);
	};
}
