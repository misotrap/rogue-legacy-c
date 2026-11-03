// Port of LevelBuilder2.cs. Method names and the order of random draws
// follow the original so the two can be compared side by side.
#include "LevelBuilder2.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace Levels
{
	namespace
	{
		const int ScreenWidth = 1320, ScreenHeight = 720; // GlobalEV
		const int MAX_ROOM_SIZE = 4;
		const bool LINK_TO_CASTLE_ONLY = true;            // LevelEV

		// EnemyType.cs
		enum : uint8_t
		{
			BallAndChain = 1, Blob = 2, BouncySpike = 3, Eagle = 4, EarthWizard = 5, Eyeball = 6, Fairy = 7,
			Fireball = 8, FireWizard = 9, Horse = 10, IceWizard = 11, Knight = 12, Ninja = 13, ShieldKnight = 14,
			Skeleton = 15, SwordKnight = 16, Turret = 17, Wall = 18, Wolf = 19, Zombie = 20, SpikeTrap = 21,
			Plant = 22, Energon = 23, Spark = 24, SkeletonArcher = 25, Chicken = 26, Platform = 27,
			HomingTurret = 28, LastBoss = 29, Dummy = 30, Starburst = 31, Portrait = 32, Mimic = 33
		};

		// LevelEV.*_ENEMY_LIST and *_ENEMY_DIFFICULTY_LIST
		const uint8_t CASTLE_ENEMY_LIST[] = { Skeleton, Knight, FireWizard, IceWizard, Eyeball, BouncySpike, SwordKnight, Zombie, Fireball, Portrait, Starburst, HomingTurret };
		const uint8_t GARDEN_ENEMY_LIST[] = { Skeleton, Blob, BallAndChain, EarthWizard, FireWizard, Eyeball, Fairy, ShieldKnight, BouncySpike, Wolf, Plant, SkeletonArcher, Starburst, Horse };
		const uint8_t TOWER_ENEMY_LIST[] = { Knight, BallAndChain, IceWizard, Eyeball, Fairy, ShieldKnight, BouncySpike, Wolf, Ninja, Plant, Fireball, SkeletonArcher, Portrait, Starburst, HomingTurret, Mimic };
		const uint8_t DUNGEON_ENEMY_LIST[] = { Skeleton, Knight, Blob, BallAndChain, EarthWizard, FireWizard, IceWizard, Eyeball, Fairy, BouncySpike, SwordKnight, Zombie, Ninja, Plant, Fireball, Starburst, HomingTurret, Horse };
		const uint8_t CASTLE_ENEMY_DIFFICULTY_LIST[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		const uint8_t GARDEN_ENEMY_DIFFICULTY_LIST[] = { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		const uint8_t TOWER_ENEMY_DIFFICULTY_LIST[] = { 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0 };
		const uint8_t DUNGEON_ENEMY_DIFFICULTY_LIST[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// LevelEV.*_ASSETSWAP_LIST: a castle object becomes the same index of
		// the area's list ("" = removed, RANDOMn = one of n variants)
		const char *const CASTLE_ASSETSWAP_LIST[] = { "BreakableBarrel1_Character", "BreakableBarrel2_Character", "CastleAssetKnightStatue_Character", "CastleAssetWindow1_Sprite", "CastleAssetWindow2_Sprite", "CastleBGPillar_Character", "CastleAssetWeb1_Sprite", "CastleAssetWeb2_Sprite", "CastleAssetBackTorch_Character", "CastleAssetSideTorch_Character", "CastleAssetChandelier1_Character", "CastleAssetChandelier2_Character", "CastleAssetCandle1_Character", "CastleAssetCandle2_Character", "CastleAssetFireplace_Character", "CastleAssetBookcase_Sprite", "CastleAssetBookCase2_Sprite", "CastleAssetBookCase3_Sprite", "CastleAssetUrn1_Character", "CastleAssetUrn2_Character", "BreakableChair1_Character", "BreakableChair2_Character", "CastleAssetTable1_Character", "CastleAssetTable2_Character", "CastleDoorOpen_Sprite", "CastleAssetFrame_Sprite" };
		const char *const DUNGEON_ASSETSWAP_LIST[] = { "BreakableCrate1_Character", "BreakableBarrel1_Character", "CastleAssetDemonStatue_Character", "DungeonSewerGrate1_Sprite", "DungeonSewerGrate2_Sprite", "", "CastleAssetWeb1_Sprite", "CastleAssetWeb2_Sprite", "DungeonChainRANDOM2_Character", "", "DungeonHangingCell1_Character", "DungeonHangingCell2_Character", "DungeonTorch1_Character", "DungeonTorch2_Character", "DungeonMaidenRANDOM3_Character", "DungeonPrison1_Sprite", "DungeonPrison2_Sprite", "DungeonPrison3_Sprite", "", "", "DungeonBucket1_Character", "DungeonBucket2_Character", "DungeonTable1_Character", "DungeonTable2_Character", "DungeonDoorOpen_Sprite", "" };
		const char *const TOWER_ASSETSWAP_LIST[] = { "BreakableCrate1_Character", "BreakableCrate2_Character", "CastleAssetAngelStatue_Character", "TowerHoleRANDOM9_Sprite", "TowerHoleRANDOM9_Sprite", "", "TowerLever1_Sprite", "TowerLever2_Sprite", "CastleAssetBackTorchUnlit_Character", "CastleAssetSideTorchUnlit_Character", "DungeonChain1_Character", "DungeonChain2_Character", "TowerTorch_Character", "TowerPedestal2_Character", "CastleAssetFireplaceNoFire_Character", "BrokenBookcase1_Sprite", "BrokenBookcase2_Sprite", "", "TowerBust1_Character", "TowerBust2_Character", "TowerChair1_Character", "TowerChair2_Character", "TowerTable1_Character", "TowerTable2_Character", "TowerDoorOpen_Sprite", "CastleAssetFrame_Sprite" };
		const char *const GARDEN_ASSETSWAP_LIST[] = { "GardenUrn1_Character", "GardenUrn2_Character", "CherubStatue_Character", "GardenFloatingRockRANDOM5_Sprite", "GardenFloatingRockRANDOM5_Sprite", "GardenPillar_Character", "", "", "GardenFairy_Character", "", "GardenVine1_Character", "GardenVine2_Character", "GardenLampPost1_Character", "GardenLampPost2_Character", "GardenFountain_Character", "GardenBush1_Sprite", "GardenBush2_Sprite", "", "", "", "GardenMushroom1_Character", "GardenMushroom2_Character", "GardenTrunk1_Character", "GardenTrunk2_Character", "GardenDoorOpen_Sprite", "" };
		const int ASSETSWAP_COUNT = (int) (sizeof(CASTLE_ASSETSWAP_LIST) / sizeof(CASTLE_ASSETSWAP_LIST[0]));

		// BossRoomType.cs and LevelEV.*_BOSS_ROOM
		const int EyeballBossRoom = 1, LastBossRoom = 2, FairyBossRoom = 5, FireballBossRoom = 6, BlobBossRoom = 7;
		const int BonusDiary = 6; // BonusRoomTypes.Diary

		// Enemies that are not IsWeighted (EnemyEV.*_IsWeighted = false, or
		// set in their constructor): only these fit a ForceFlying orb.
		bool IsWeighted(int type)
		{
			switch (type)
			{
			case BouncySpike: case Chicken: case EarthWizard: case Eyeball: case Fairy: case FireWizard:
			case Fireball: case IceWizard: case Portrait: case Spark: case SpikeTrap: case Starburst:
			case Dummy: case Wall: case Platform: case Turret: case Eagle: case LastBoss:
				return false;
			default:
				return true;
			}
		}

		int TypeIndex(LevelType t) { return (int) t; }

		// XNA Rectangle.Intersects: touching edges do not count
		bool Intersects(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
		{
			return bx < ax + aw && ax < bx + bw && by < ay + ah && ay < by + bh;
		}

		const char *SwapListName(LevelType t, int i)
		{
			switch (t)
			{
			case LevelType::DUNGEON: return DUNGEON_ASSETSWAP_LIST[i];
			case LevelType::TOWER: return TOWER_ASSETSWAP_LIST[i];
			case LevelType::GARDEN: return GARDEN_ASSETSWAP_LIST[i];
			default: return CASTLE_ASSETSWAP_LIST[i];
			}
		}

		// RoomObj.ReverseObjNames: door-rule names swap left and right
		void ReverseName(std::string &n)
		{
			static const char *const pairs[][2] = {
				{ "Left", "Right" }, { "!Left", "!Right" }, { "!RightTop", "!LeftTop" }, { "!RightBottom", "!LeftBottom" },
				{ "RightTop", "LeftTop" }, { "RightBottom", "LeftBottom" }, { "!BottomLeft", "!BottomRight" },
				{ "!TopLeft", "!TopRight" }, { "BottomLeft", "BottomRight" }, { "TopLeft", "TopRight" },
			};
			for (const auto &p : pairs)
			{
				if (n == p[0]) { n = p[1]; return; }
				if (n == p[1]) { n = p[0]; return; }
			}
		}

		bool IsRectObj(const ObjData &o)
		{
			return o.type == "CollHullObj" || o.type == "HazardObj" || o.type == "ChestObj" || o.type == "BorderObj";
		}

		// RoomObj.Reverse for one rectangle-like object: mirrored about the
		// room centre (rotated ones by their upper-right corner)
		void MirrorRect(ObjData &o, float roomWidth)
		{
			if (o.rotation == 0)
			{
				o.x = roomWidth - o.x - o.width;
			}
			else
			{
				float a = o.rotation * 3.14159265f / 180.0f;
				float urx = o.x + o.width * cosf(a), ury = o.y + o.width * sinf(a);
				o.x = roomWidth - urx;
				o.y = ury;
				o.rotation = -o.rotation;
			}
		}

		// LevelBuilder2.RemoveFromListHelper: objects named after a door
		// position only exist when that door is there ("Top"), or not ("!Top")
		struct DoorFlags
		{
			bool top, bottom, left, right;
			bool topLeft, topRight, bottomLeft, bottomRight;
			bool rightTop, rightBottom, leftTop, leftBottom;
		};

		bool Has(const std::string &n, const char *s) { return n.find(s) != std::string::npos; }

		bool RuleRemoves(const std::string &n, bool has, const char *word)
		{
			std::string neg = std::string("!") + word;
			return (!has && Has(n, word) && !Has(n, neg.c_str())) || (has && Has(n, neg.c_str()));
		}

		bool RuleRemovesExact(const std::string &n, bool has, const char *word)
		{
			std::string neg = std::string("!") + word;
			size_t len = strlen(word);
			return (!has && Has(n, word) && !Has(n, neg.c_str()) && n.size() == len) || (has && Has(n, neg.c_str()) && n.size() == len + 1);
		}

		bool RemovedByDoors(const std::string &n, const DoorFlags &f)
		{
			if (n.empty()) return false;
			return RuleRemoves(n, f.topLeft, "TopLeft") || RuleRemoves(n, f.topRight, "TopRight") || RuleRemovesExact(n, f.top, "Top")
				|| RuleRemoves(n, f.bottomLeft, "BottomLeft") || RuleRemoves(n, f.bottomRight, "BottomRight") || RuleRemovesExact(n, f.bottom, "Bottom")
				|| RuleRemoves(n, f.leftTop, "LeftTop") || RuleRemoves(n, f.leftBottom, "LeftBottom") || RuleRemovesExact(n, f.left, "Left")
				|| RuleRemoves(n, f.rightTop, "RightTop") || RuleRemoves(n, f.rightBottom, "RightBottom") || RuleRemovesExact(n, f.right, "Right");
		}

		ObjData Terrain(float x, float y, float w, float h)
		{
			ObjData o;
			o.type = "CollHullObj";
			o.x = x; o.y = y; o.width = w; o.height = h;
			return o;
		}

		// BorderObj: the Collides* flags say which sides show a border
		ObjData Border(float x, float y, float w, float h, DoorPos side)
		{
			ObjData o;
			o.type = "BorderObj";
			o.x = x; o.y = y; o.width = w; o.height = h;
			o.collidesTop = side == DoorPos::Top;
			o.collidesBottom = side == DoorPos::Bottom;
			o.collidesLeft = side == DoorPos::Left;
			o.collidesRight = side == DoorPos::Right;
			return o;
		}

		const char *DifficultyName(int d)
		{
			static const char *const names[] = { "BASIC", "ADVANCED", "EXPERT", "MINIBOSS" };
			return names[d >= 0 && d < 4 ? d : 0];
		}
	}

	const AreaInfo *DefaultAreas()
	{
		// Game.cs: CastleZone, GardenZone, TowerZone, DungeonZone
		static const AreaInfo areas[4] = {
			{ LevelType::CASTLE, 24, 28, 1, 3, 2, 3, true },
			{ LevelType::GARDEN, 23, 27, 1, 3, 2, 3, true },
			{ LevelType::TOWER, 23, 27, 1, 3, 2, 3, true },
			{ LevelType::DUNGEON, 23, 27, 1, 3, 2, 3, true },
		};
		return areas;
	}

	LevelBuilder2::LevelBuilder2(const std::vector<RoomTemplate> &templates) : T(templates)
	{
		Index();
	}

	int LevelBuilder2::RandomInt(int min, int max)
	{
		if (max < min) return min;
		return std::uniform_int_distribution<int>(min, max)(rng);
	}

	// LevelParser.ParseRooms + StoreRoom + StoreSpecialRoom
	void LevelBuilder2::Index()
	{
		static const char *const special[] = { "Start", "Linker", "Boss", "EntranceBoss", "Secret", "Bonus", "CastleEntrance",
			"Throne", "Tutorial", "Ending", "Compass", "DEBUG_ROOM", "ChallengeBoss" };
		static const LevelType order[] = { LevelType::CASTLE, LevelType::DUNGEON, LevelType::GARDEN, LevelType::TOWER };

		templDoors[0].resize(T.size());
		templDoors[1].resize(T.size());
		for (size_t i = 0; i < T.size(); i++)
		{
			const RoomTemplate &r = T[i];
			for (const ObjData &o : r.objs)
			{
				if (o.type != "DoorObj") continue;
				Door d;
				d.pos = o.doorPos;
				d.x = o.x; d.y = o.y;
				d.width = (int) o.width; d.height = (int) o.height;
				d.bossDoor = o.bossDoor;
				templDoors[0][i].push_back(d);
				// RoomObj.Reverse
				if (d.pos == DoorPos::Left) { d.x = (float) r.width - d.width; d.pos = DoorPos::Right; }
				else if (d.pos == DoorPos::Right) { d.x = 0; d.pos = DoorPos::Left; }
				else if (d.pos == DoorPos::Top || d.pos == DoorPos::Bottom) d.x = (float) r.width - d.x - d.width;
				templDoors[1][i].push_back(d);
			}
		}

		for (size_t i = 0; i < T.size(); i++)
		{
			const RoomTemplate &r = T[i];
			int ti = (int) i;
			if (r.x >= 10000) continue;
			bool debug = false;
			for (const ObjData &o : r.objs)
				if (o.type == "PlayerStartObj") debug = true;
			if (debug) continue; // test rooms
			if (r.name == "Boss") { bossRooms.push_back(ti); continue; }
			if (r.name == "ChallengeBoss") { challengeRooms.push_back(ti); continue; }

			bool isSpecial = false;
			for (const char *s : special)
				if (r.name == s) isSpecial = true;

			for (LevelType t : order)
			{
				if (!r.InPool(t)) continue;
				int k = TypeIndex(t);
				if (!isSpecial)
				{
					if (r.width % ScreenWidth != 0 || r.height % ScreenHeight != 0 || r.width <= 0 || r.height <= 0)
					{
						fprintf(stderr, "room %s in %s is not a whole number of screens, skipped\n", r.name.c_str(), r.mapFile.c_str());
						break;
					}
					int w = r.width / ScreenWidth, h = r.height / ScreenHeight;
					if (w > MAX_ROOM_SIZE || h > MAX_ROOM_SIZE) break;
					std::vector<Candidate> &pool = r.isDLCMap ? dlcPool[k] : areaPool[k][w - 1][h - 1];
					pool.push_back({ ti, false });
					pool.push_back({ ti, true });
					continue;
				}
				if (r.name == "Start") { if (startRoom < 0) startRoom = ti; }
				else if (r.name == "Linker") linker[k] = ti;
				else if (r.name == "EntranceBoss") bossEntrance[k] = ti;
				else if (r.name == "CastleEntrance") { if (castleEntrance < 0) castleEntrance = ti; }
				else if (r.name == "Compass") { if (compassRoom < 0) compassRoom = ti; }
				else if (r.name == "Secret") { secretPool[k].push_back({ ti, false }); secretPool[k].push_back({ ti, true }); }
				else if (r.name == "Bonus") { bonusPool[k].push_back({ ti, false }); bonusPool[k].push_back({ ti, true }); }
				else if (r.name == "Tutorial") { if (tutorialRoom < 0) tutorialRoom = ti; }
				else if (r.name == "Throne") { if (throneRoom < 0) throneRoom = ti; }
				else if (r.name == "Ending") { if (endingRoom < 0) endingRoom = ti; }
			}
		}
	}

	PlacedRoom LevelBuilder2::Instantiate(const Candidate &c, LevelType type, float x, float y) const
	{
		const RoomTemplate &r = T[c.templ];
		PlacedRoom p;
		p.templ = c.templ;
		p.reversed = c.reversed;
		p.levelType = type;
		p.x = x; p.y = y;
		p.width = r.width; p.height = r.height;
		p.name = r.name; p.tag = r.tag;
		p.doors = DoorsOf(c);
		for (Door &d : p.doors)
		{
			d.x += x;
			d.y += y;
		}
		return p;
	}

	void LevelBuilder2::MoveRoom(PlacedRoom &room, float x, float y)
	{
		float dx = x - room.x, dy = y - room.y;
		room.x = x;
		room.y = y;
		for (Door &d : room.doors)
		{
			d.x += dx;
			d.y += dy;
		}
	}

	// Where a room joining 'needsLinking' through its door 'd' would go
	static void LinkedPosition(const Door &needsLinking, const PlacedRoom &needsRoom, const Door &d, int w, int h, bool forCollision, float &x, float &y)
	{
		switch (needsLinking.pos)
		{
		case DoorPos::Left:
			x = (forCollision ? needsRoom.x : needsLinking.x) - w;
			y = needsLinking.y - d.y;
			break;
		case DoorPos::Right:
			x = needsLinking.x + needsLinking.width;
			y = needsLinking.y - d.y;
			break;
		case DoorPos::Top:
			x = needsLinking.x - d.x;
			y = needsLinking.y - h;
			break;
		default:
			x = needsLinking.x - d.x;
			y = needsLinking.y + needsLinking.height;
			break;
		}
	}

	bool LevelBuilder2::CheckForRoomCollision(const Door &needsLinking, const PlacedRoom &needsRoom,
		const std::vector<PlacedRoom> &rooms, const Candidate &c, int door) const
	{
		const RoomTemplate &r = T[c.templ];
		float x, y;
		LinkedPosition(needsLinking, needsRoom, DoorsOf(c)[door], r.width, r.height, true, x, y);
		if (x < 0) return true; // Do not allow rooms be made past xPos = 0.
		for (const PlacedRoom &o : rooms)
		{
			if (Intersects(o.x, o.y, (float) o.width, (float) o.height, x, y, (float) r.width, (float) r.height))
				return true;
		}
		return false;
	}

	void LevelBuilder2::FindSuitableDoors(const Door &needsLinking, const PlacedRoom &needsRoom, const std::vector<PlacedRoom> &rooms,
		LevelType type, bool findRoomsWithMoreDoors, std::vector<SuitableDoor> &out) const
	{
		DoorPos wanted = Opposite(needsLinking.pos);
		int k = TypeIndex(type);
		auto scan = [&](const std::vector<Candidate> &pool)
		{
			for (const Candidate &c : pool)
			{
				const std::vector<Door> &doors = DoorsOf(c);
				if (findRoomsWithMoreDoors && doors.size() <= 1) continue;
				for (int d = 0; d < (int) doors.size(); d++)
				{
					if (doors[d].pos == wanted && !CheckForRoomCollision(needsLinking, needsRoom, rooms, c, d))
						out.push_back({ c, d });
				}
			}
		};
		for (int i = 1; i <= MAX_ROOM_SIZE; i++)
			for (int j = 1; j <= MAX_ROOM_SIZE; j++)
				scan(areaPool[k][i - 1][j - 1]);
		scan(dlcPool[k]);
	}

	void LevelBuilder2::FindSuitableDoors(const Door &needsLinking, const PlacedRoom &needsRoom, const std::vector<Candidate> &pool,
		const std::vector<PlacedRoom> &rooms, std::vector<SuitableDoor> &out) const
	{
		DoorPos wanted = Opposite(needsLinking.pos);
		for (const Candidate &c : pool)
		{
			const RoomTemplate &r = T[c.templ];
			if (diaryComplete && r.name == "Bonus" && atoi(r.tag.c_str()) == BonusDiary) continue;
			const std::vector<Door> &doors = DoorsOf(c);
			for (int d = 0; d < (int) doors.size(); d++)
			{
				if (doors[d].pos == wanted && !CheckForRoomCollision(needsLinking, needsRoom, rooms, c, d))
					out.push_back({ c, d });
			}
		}
	}

	bool LevelBuilder2::CreateArea(int areaSize, const AreaInfo &area, std::vector<PlacedRoom> &rooms,
		int startingRoom, bool firstRoom, std::vector<int> &areaRooms)
	{
		bool bossAdded = false;
		float bossRoomChance = -100;
		float bossRoomChanceIncrease = 100.0f / areaSize;
		if (area.bossInArea) bossRoomChance = 0;
		else bossAdded = true;

		int k = TypeIndex(area.levelType);
		int secretRoomsNeeded = RandomInt(area.secretRoomsMin, area.secretRoomsMax);
		int secretRoomIncrease = areaSize / (secretRoomsNeeded + 1);
		int nextSecretRoomToAdd = secretRoomIncrease;
		std::vector<Candidate> secretCopy = secretPool[k];
		int bonusRoomsNeeded = RandomInt(area.bonusRoomsMin, area.bonusRoomsMax);
		int bonusRoomIncrease = areaSize / (bonusRoomsNeeded + 1);
		int nextBonusRoomToAdd = bonusRoomIncrease;
		std::vector<Candidate> bonusCopy = bonusPool[k];

		int numRoomsLeftToCreate = areaSize;
		int doorPercent[5] = { 0, 0, 0, 0, 0 }; // indexed by DoorPos
		DoorPos startingDoorPosition = DoorPos::None;
		switch (area.levelType)
		{
		case LevelType::CASTLE:
			doorPercent[1] = 90; doorPercent[2] = 90; doorPercent[3] = 90; doorPercent[4] = 90;
			startingDoorPosition = DoorPos::Right;
			break;
		case LevelType::GARDEN:
			doorPercent[1] = 70; doorPercent[2] = 100; doorPercent[3] = 45; doorPercent[4] = 45;
			startingDoorPosition = DoorPos::Right;
			break;
		case LevelType::TOWER:
			doorPercent[1] = 45; doorPercent[2] = 45; doorPercent[3] = 100; doorPercent[4] = 60;
			startingDoorPosition = DoorPos::Top;
			break;
		case LevelType::DUNGEON:
			doorPercent[1] = 55; doorPercent[2] = 55; doorPercent[3] = 45; doorPercent[4] = 100;
			startingDoorPosition = DoorPos::Bottom;
			break;
		default:
			break;
		}

		if (firstRoom)
		{
			rooms.push_back(Instantiate({ startRoom, false }, LevelType::NONE, 0, 0));
			int start = (int) rooms.size() - 1;
			areaRooms.push_back(start);
			numRoomsLeftToCreate--;
			const RoomTemplate &e = T[castleEntrance];
			rooms.push_back(Instantiate({ castleEntrance, false }, LevelType::CASTLE,
				rooms[start].x + rooms[start].width, rooms[start].Bottom() - e.height));
			startingRoom = (int) rooms.size() - 1;
			areaRooms.push_back(startingRoom);
			numRoomsLeftToCreate--;
		}

		std::vector<DoorRef> doorsToLink;
		DoorRef startingDoor = { -1, -1 };
		for (int d = 0; d < (int) rooms[startingRoom].doors.size(); d++)
		{
			if (rooms[startingRoom].doors[d].pos == startingDoorPosition)
			{
				startingDoor = { startingRoom, d };
				doorsToLink.push_back(startingDoor);
				break;
			}
		}
		if (doorsToLink.empty())
		{
			fprintf(stderr, "The starting room does not have a %s door\n", DoorPosName(startingDoorPosition));
			return false;
		}
		const float startWidth = (float) T[startRoom].width;

		std::vector<SuitableDoor> suitable;
		while (numRoomsLeftToCreate > 0)
		{
			if (doorsToLink.empty()) break; // ran out of rooms to make
			DoorRef needs = doorsToLink[0];
			bool isStartingDoor = needs.room == startingDoor.room && needs.door == startingDoor.door;
			bool linkDoor = false;
			if ((doorsToLink.size() <= 5 && !isStartingDoor && numRoomsLeftToCreate > 0) || isStartingDoor)
				linkDoor = true;
			else
			{
				int percentChance = doorPercent[(int) rooms[needs.room].doors[needs.door].pos];
				if (percentChance - RandomInt(1, 100) >= 0) linkDoor = true;
			}
			if (!linkDoor)
			{
				doorsToLink.erase(doorsToLink.begin());
				continue;
			}

			// Copies: 'rooms' grows below and would invalidate references
			Door needsDoor = rooms[needs.room].doors[needs.door];
			PlacedRoom needsRoom = rooms[needs.room];
			needsRoom.doors.clear();
			suitable.clear();

			bool addingBossRoom = false;
			if (bossRoomChance >= RandomInt(50, 100) && !bossAdded)
			{
				int entrance = bossEntrance[k];
				addingBossRoom = true;
				if (entrance >= 0)
				{
					const std::vector<Door> &doors = templDoors[0][entrance];
					for (int d = 0; d < (int) doors.size(); d++)
					{
						if (doors[d].pos == Opposite(needsDoor.pos) && !CheckForRoomCollision(needsDoor, needsRoom, rooms, { entrance, false }, d))
						{
							bossAdded = true;
							suitable.push_back({ { entrance, false }, d });
							break;
						}
					}
				}
			}
			else
				bossRoomChance += bossRoomChanceIncrease;

			bool addingSpecialRoom = false, addingSecretRoom = false;
			if ((addingBossRoom && !bossAdded) || !addingBossRoom)
			{
				suitable.clear();
				if ((int) areaRooms.size() >= nextSecretRoomToAdd && secretRoomsNeeded > 0)
				{
					addingSpecialRoom = true;
					addingSecretRoom = true;
					FindSuitableDoors(needsDoor, needsRoom, secretCopy, rooms, suitable);
				}
				else if ((int) areaRooms.size() >= nextBonusRoomToAdd && bonusRoomsNeeded > 0)
				{
					addingSpecialRoom = true;
					FindSuitableDoors(needsDoor, needsRoom, bonusCopy, rooms, suitable);
				}
				if (!addingSpecialRoom || suitable.empty())
				{
					// Early on, avoid rooms with a single door (dead ends)
					addingSpecialRoom = false;
					suitable.clear();
					FindSuitableDoors(needsDoor, needsRoom, rooms, area.levelType, areaRooms.size() < 5, suitable);
				}
				else if (addingSecretRoom)
				{
					nextSecretRoomToAdd = (int) areaRooms.size() + secretRoomIncrease;
					secretRoomsNeeded--;
				}
				else
				{
					nextBonusRoomToAdd = (int) areaRooms.size() + bonusRoomIncrease;
					bonusRoomsNeeded--;
				}
			}

			if (suitable.empty())
			{
				doorsToLink.erase(doorsToLink.begin());
				continue;
			}

			int randomDoorIndex = RandomInt(0, (int) suitable.size() - 1);
			std::shuffle(suitable.begin(), suitable.end(), rng); // CDGMath.Shuffle
			SuitableDoor chosen = suitable[randomDoorIndex];
			if (addingSpecialRoom)
			{
				std::vector<Candidate> &copy = addingSecretRoom ? secretCopy : bonusCopy;
				auto it = std::find(copy.begin(), copy.end(), chosen.cand);
				if (it != copy.end()) copy.erase(it);
			}

			const RoomTemplate &r = T[chosen.cand.templ];
			float x, y;
			LinkedPosition(needsDoor, needsRoom, DoorsOf(chosen.cand)[chosen.door], r.width, r.height, false, x, y);
			rooms.push_back(Instantiate(chosen.cand, area.levelType, x, y));
			int added = (int) rooms.size() - 1;
			areaRooms.push_back(added);
			numRoomsLeftToCreate--;
			doorsToLink.erase(doorsToLink.begin());
			for (int d = 0; d < (int) rooms[added].doors.size(); d++)
			{
				// Prevents the creation of rooms above the starting room
				if (d != chosen.door && rooms[added].doors[d].x >= startWidth)
					doorsToLink.push_back({ added, d });
			}
			rooms[added].doors[chosen.door].attached = true;
			rooms[needs.room].doors[needs.door].attached = true;
		}
		return true;
	}

	bool LevelBuilder2::FindFurthestDoor(const std::vector<PlacedRoom> &rooms, const std::vector<int> &subset, DoorPos direction,
		DoorPos doorWanted, bool castleOnly, DoorRef &found) const
	{
		if (subset.empty()) return false;
		DoorPos oppositeOfDoorWanted = Opposite(doorWanted);
		const PlacedRoom &start = rooms[subset[0]];
		float furthestDistance = -10;
		found = { -1, -1 };
		for (int ri : subset)
		{
			const PlacedRoom &room = rooms[ri];
			if (ri == subset[0] || (castleOnly && room.levelType != LevelType::CASTLE)) continue;
			float distance = 0;
			switch (direction)
			{
			case DoorPos::Right: distance = room.x - start.x; break;
			case DoorPos::Left: distance = start.x - room.x; break;
			case DoorPos::Top: distance = start.y - room.y; break;
			default: distance = room.y - start.y; break;
			}
			if (distance < furthestDistance) continue;
			for (int d = 0; d < (int) room.doors.size(); d++)
			{
				const Door &door = room.doors[d];
				if (door.pos == DoorPos::None) continue;
				if (door.pos != doorWanted && door.pos == oppositeOfDoorWanted) continue;
				// The door must lead nowhere yet: no other room right behind it
				bool addRoom = true;
				for (int ci : subset)
				{
					const PlacedRoom &other = rooms[ci];
					if (ci != ri && Intersects(other.x - 10, other.y - 10, (float) other.width + 20, (float) other.height + 20,
						door.x, door.y, (float) door.width, (float) door.height))
					{
						addRoom = false;
						break;
					}
				}
				if (addRoom)
				{
					found = { ri, d };
					furthestDistance = distance;
					break;
				}
			}
		}
		return found.room >= 0;
	}

	bool LevelBuilder2::AddLinkerToRoom(std::vector<PlacedRoom> &rooms, DoorRef needsLinking, DoorPos doorWanted, DoorRef &result)
	{
		LevelType type = rooms[needsLinking.room].levelType;
		int lk = linker[TypeIndex(type)];
		if (lk < 0) return false;
		Door needsDoor = rooms[needsLinking.room].doors[needsLinking.door];
		PlacedRoom needsRoom = rooms[needsLinking.room];
		const std::vector<Door> &doors = templDoors[0][lk];
		int linkTo = -1;
		for (int d = 0; d < (int) doors.size(); d++)
		{
			if (doors[d].pos == Opposite(needsDoor.pos)) { linkTo = d; break; }
		}
		if (linkTo < 0) return false;
		float x, y;
		LinkedPosition(needsDoor, needsRoom, doors[linkTo], T[lk].width, T[lk].height, false, x, y);
		rooms.push_back(Instantiate({ lk, false }, type, x, y));
		int added = (int) rooms.size() - 1;
		rooms[needsLinking.room].doors[needsLinking.door].attached = true;
		rooms[added].doors[linkTo].attached = true;
		result = { -1, -1 };
		for (int d = 0; d < (int) rooms[added].doors.size(); d++)
			if (rooms[added].doors[d].pos == doorWanted) result = { added, d };
		return result.room >= 0;
	}

	bool LevelBuilder2::CreateLevel(Castle &castle, const AreaInfo *areas, int numAreas, uint32_t seed)
	{
		if (startRoom < 0 || castleEntrance < 0 || compassRoom < 0)
		{
			fprintf(stderr, "LevelBuilder2: missing special rooms (start %d, castle entrance %d, compass %d)\n", startRoom, castleEntrance, compassRoom);
			return false;
		}
		rng.seed(seed);

		std::vector<const AreaInfo *> sequential, nonSequential;
		for (int i = 0; i < numAreas; i++)
		{
			if (areas[i].levelType == LevelType::CASTLE || areas[i].levelType == LevelType::GARDEN)
				sequential.push_back(&areas[i]);
			else
				nonSequential.push_back(&areas[i]);
		}

		std::vector<PlacedRoom> master;
		for (int attempt = 0; attempt < 100; attempt++) // restartAll
		{
			master.clear();
			bool restartAll = false;

			for (size_t i = 0; i < sequential.size() && !restartAll; i++)
			{
				const AreaInfo &area = *sequential[i];
				bool done = false;
				for (int seqAttempt = 0; seqAttempt < 100 && !done && !restartAll; seqAttempt++) // restartSequential
				{
					int numRooms = RandomInt(area.totalRoomsMin, area.totalRoomsMax);
					int creationCounter = 0;
					std::vector<PlacedRoom> work;
					std::vector<int> areaRooms;
					bool addedBossRoom = false;
					bool created = false;
					while (!created || (int) areaRooms.size() < numRooms || !addedBossRoom)
					{
						addedBossRoom = !area.bossInArea;
						work = master;
						areaRooms.clear();
						if (i == 0)
						{
							CreateArea(numRooms, area, work, -1, true, areaRooms);
						}
						else
						{
							std::vector<int> all(work.size());
							for (size_t j = 0; j < work.size(); j++) all[j] = (int) j;
							DoorRef furthest, linkerDoor;
							if (!FindFurthestDoor(work, all, DoorPos::Right, DoorPos::Right, LINK_TO_CASTLE_ONLY, furthest)
								|| !AddLinkerToRoom(work, furthest, DoorPos::Right, linkerDoor))
							{
								restartAll = true;
								break;
							}
							CreateArea(numRooms, area, work, linkerDoor.room, false, areaRooms);
						}
						created = true;
						for (int ri : areaRooms)
							if (work[ri].name == "EntranceBoss") addedBossRoom = true;
						if (++creationCounter > 15) { restartAll = true; break; }
					}
					if (restartAll) break;
					DoorRef dummy;
					if (FindFurthestDoor(work, areaRooms, DoorPos::Right, DoorPos::Right, false, dummy)
						&& FindFurthestDoor(work, areaRooms, DoorPos::Top, DoorPos::Top, false, dummy)
						&& FindFurthestDoor(work, areaRooms, DoorPos::Bottom, DoorPos::Bottom, false, dummy))
					{
						master = work;
						done = true;
					}
				}
				if (!done) restartAll = true;
			}

			for (size_t i = 0; i < nonSequential.size() && !restartAll; i++)
			{
				const AreaInfo &area = *nonSequential[i];
				DoorPos furthestDoorDirection = area.levelType == LevelType::TOWER ? DoorPos::Top : DoorPos::Bottom;
				bool done = false;
				for (int nsAttempt = 0; nsAttempt < 100 && !done && !restartAll; nsAttempt++) // restartNonSequential
				{
					int numRooms = RandomInt(area.totalRoomsMin, area.totalRoomsMax);
					int creationCounter = 0;
					std::vector<PlacedRoom> work;
					std::vector<int> areaRooms;
					bool addedBossRoom = false;
					bool created = false;
					while (!created || (int) areaRooms.size() < numRooms || !addedBossRoom)
					{
						addedBossRoom = !area.bossInArea;
						work = master;
						areaRooms.clear();
						std::vector<int> all(work.size());
						for (size_t j = 0; j < work.size(); j++) all[j] = (int) j;
						DoorRef furthest, linkerDoor;
						if (!FindFurthestDoor(work, all, furthestDoorDirection, furthestDoorDirection, LINK_TO_CASTLE_ONLY, furthest)
							|| !AddLinkerToRoom(work, furthest, furthestDoorDirection, linkerDoor))
						{
							restartAll = true;
							break;
						}
						CreateArea(numRooms, area, work, linkerDoor.room, false, areaRooms);
						created = true;
						if (++creationCounter > 15) { restartAll = true; break; }
						for (int ri : areaRooms)
							if (work[ri].name == "EntranceBoss") addedBossRoom = true;
					}
					if (restartAll) break;
					DoorRef dummy;
					bool ok = true;
					if (area.levelType == LevelType::TOWER)
						ok = FindFurthestDoor(work, areaRooms, DoorPos::Right, DoorPos::Right, false, dummy)
							&& FindFurthestDoor(work, areaRooms, DoorPos::Top, DoorPos::Top, false, dummy);
					else if (area.levelType == LevelType::DUNGEON)
						ok = FindFurthestDoor(work, areaRooms, DoorPos::Right, DoorPos::Right, false, dummy)
							&& FindFurthestDoor(work, areaRooms, DoorPos::Bottom, DoorPos::Bottom, false, dummy);
					if (ok)
					{
						master = work;
						done = true;
					}
				}
				if (!done) restartAll = true;
			}

			if (!restartAll) break;
			if (attempt == 99)
			{
				fprintf(stderr, "LevelBuilder2: could not build a castle\n");
				return false;
			}
		}

		CloseRemainingDoors(master);
		AddDoorBorders(master);     // after all doors are closed
		AddBottomPlatforms(master); // after all doors are closed
		LinkAllBossEntrances(master);
		PlacedRoom compass = Instantiate({ compassRoom, false }, LevelType::NONE, -999999, -999999);
		master.push_back(compass); // the last room is always the compass room
		for (PlacedRoom &room : master)
			Materialize(room); // AddRemoveExtraObjects + AddProceduralEnemies
		castle.rooms = std::move(master);
		return true;
	}

	void LevelBuilder2::CloseRemainingDoors(std::vector<PlacedRoom> &rooms)
	{
		struct Ref { int room, door; };
		std::vector<Ref> all, open;
		for (int r = 0; r < (int) rooms.size(); r++)
			for (int d = 0; d < (int) rooms[r].doors.size(); d++)
				if (rooms[r].doors[d].pos != DoorPos::None)
				{
					all.push_back({ r, d });
					if (!rooms[r].doors[d].attached) open.push_back({ r, d });
				}

		std::vector<Ref> toRemove;
		for (const Ref &f : open)
		{
			Door &first = rooms[f.room].doors[f.door];
			bool removeDoor = true;
			for (const Ref &s : all)
			{
				if (s.room == f.room && s.door == f.door) continue;
				const Door &second = rooms[s.room].doors[s.door];
				float sx = second.x, sy = second.y, sw = (float) second.width, sh = (float) second.height;
				float fw = (float) first.width, fh = (float) first.height;
				switch (first.pos)
				{
				case DoorPos::Left:
					if (second.x < first.x && Intersects(first.x - 5, first.y, fw, fh, sx, sy, sw, sh)) removeDoor = false;
					break;
				case DoorPos::Right:
					if (second.x > first.x && Intersects(first.x + 5, first.y, fw, fh, sx, sy, sw, sh)) removeDoor = false;
					break;
				case DoorPos::Top:
					if (second.y < first.y && Intersects(first.x, first.y - 5, fw, fh, sx, sy, sw, sh)) removeDoor = false;
					break;
				case DoorPos::Bottom:
					if (second.y > first.y && Intersects(first.x - 5, first.y + 5, fw, fh, sx, sy, sw, sh)) removeDoor = false;
					break;
				default:
					removeDoor = false;
					break;
				}
			}
			if (removeDoor) toRemove.push_back(f);
			else first.attached = true;
		}

		// RemoveDoorFromRoom: a wall and its border replace the door.
		// Highest door index first so the remaining indices stay valid.
		std::sort(toRemove.begin(), toRemove.end(), [](const Ref &a, const Ref &b)
		{
			return a.room != b.room ? a.room < b.room : a.door > b.door;
		});
		for (const Ref &f : toRemove)
		{
			PlacedRoom &room = rooms[f.room];
			const Door door = room.doors[f.door];
			float x = door.x - room.x, y = door.y - room.y;
			room.extra.push_back(Terrain(x, y, (float) door.width, (float) door.height));
			DoorPos side = door.pos == DoorPos::Left ? DoorPos::Right : door.pos == DoorPos::Right ? DoorPos::Left
				: door.pos == DoorPos::Top ? DoorPos::Bottom : DoorPos::Top;
			room.extra.push_back(Border(x, y, (float) door.width, (float) door.height, side));
			room.doors.erase(room.doors.begin() + f.door);
		}
	}

	void LevelBuilder2::AddDoorBorders(std::vector<PlacedRoom> &rooms)
	{
		for (PlacedRoom &room : rooms)
		{
			for (const Door &door : room.doors)
			{
				float x = door.x - room.x, y = door.y - room.y;
				switch (door.pos)
				{
				case DoorPos::Left:
				case DoorPos::Right:
					// -60 because a grid tile is 60 large
					room.extra.push_back(Border(x, y - 60, (float) door.width, 60, DoorPos::Bottom));
					room.extra.push_back(Border(x, y + door.height, (float) door.width, 60, DoorPos::Top));
					break;
				case DoorPos::Top:
				case DoorPos::Bottom:
					room.extra.push_back(Border(x + door.width, y, 60, (float) door.height, DoorPos::Left));
					room.extra.push_back(Border(x - 60, y, 60, (float) door.height, DoorPos::Right));
					break;
				default:
					break;
				}
			}
		}
	}

	void LevelBuilder2::AddBottomPlatforms(std::vector<PlacedRoom> &rooms)
	{
		for (PlacedRoom &room : rooms)
		{
			for (const Door &door : room.doors)
			{
				if (door.pos != DoorPos::Bottom) continue;
				// A one-way platform over the hole: half a grid tile (30) high
				ObjData p = Terrain(door.x - room.x, door.y - room.y, (float) door.width, 30);
				p.collidesBottom = p.collidesLeft = p.collidesRight = false;
				room.extra.push_back(p);
				room.extra.push_back(Border(p.x, p.y, p.width, p.height, DoorPos::Top));
			}
		}
	}

	void LevelBuilder2::LinkAllBossEntrances(std::vector<PlacedRoom> &rooms)
	{
		auto specific = [](const std::vector<int> &list, const std::vector<RoomTemplate> &T, int type)
		{
			for (int t : list)
				if (!T[t].tag.empty() && atoi(T[t].tag.c_str()) == type) return t;
			return -1;
		};
		float nextX = -100000; // boss rooms float left of the castle
		std::vector<PlacedRoom> bossToAdd, challengeToAdd;
		int count = (int) rooms.size();
		for (int i = 0; i < count; i++)
		{
			PlacedRoom &room = rooms[i];
			int bossType = 0;
			switch (room.levelType)
			{
			case LevelType::CASTLE: bossType = EyeballBossRoom; break;
			case LevelType::TOWER: bossType = FireballBossRoom; break;
			case LevelType::DUNGEON: bossType = BlobBossRoom; break;
			case LevelType::GARDEN: bossType = FairyBossRoom; break;
			default: break;
			}
			if (room.name == "EntranceBoss")
			{
				int b = specific(bossRooms, T, bossType);
				if (b < 0 && !bossRooms.empty()) b = bossRooms[RandomInt(0, (int) bossRooms.size() - 1)];
				if (b < 0) continue;
				PlacedRoom boss = Instantiate({ b, false }, room.levelType, nextX, 0);
				for (Door &d : boss.doors)
					if (d.bossDoor) d.locked = true; // no leaving a boss fight
				nextX += boss.width;
				room.linkedRoom = count + (int) bossToAdd.size();
				boss.linkedRoom = i;
				bossToAdd.push_back(boss);
				int c = specific(challengeRooms, T, bossType);
				if (c >= 0)
				{
					PlacedRoom ch = Instantiate({ c, false }, room.levelType, 0, 0);
					ch.linkedRoom = i;
					challengeToAdd.push_back(ch);
				}
			}
			else if (room.name == "CastleEntrance" && tutorialRoom >= 0)
			{
				int b = specific(bossRooms, T, LastBossRoom);
				if (b < 0) continue;
				PlacedRoom tutorial = Instantiate({ tutorialRoom, false }, LevelType::NONE, 100000, -100000);
				PlacedRoom boss = Instantiate({ b, false }, LevelType::NONE, 150000, -100000);
				int ti = count + (int) bossToAdd.size() + 1;
				room.linkedRoom = ti;
				tutorial.linkedRoom = ti - 1;
				boss.linkedRoom = ti;
				bossToAdd.push_back(boss);
				bossToAdd.push_back(tutorial);
			}
		}
		int c = specific(challengeRooms, T, LastBossRoom);
		if (c >= 0) challengeToAdd.push_back(Instantiate({ c, false }, LevelType::CASTLE, 0, 0));

		// ConvertChallengeBossRooms: challenge rooms float far away
		for (PlacedRoom &ch : challengeToAdd)
		{
			int type = atoi(ch.tag.c_str());
			float y = type == FairyBossRoom ? -6000000.0f : type == FireballBossRoom ? -7000000.0f
				: type == BlobBossRoom ? -8000000.0f : type == LastBossRoom ? -9000000.0f
				: type == EyeballBossRoom ? 5000000.0f : -5000000.0f;
			MoveRoom(ch, 0, y);
			for (Door &d : ch.doors)
				if (d.bossDoor) d.locked = true;
		}
		for (PlacedRoom &b : bossToAdd) rooms.push_back(b);
		for (PlacedRoom &ch : challengeToAdd) rooms.push_back(ch);
		/* The entrance's boss door leads to its challenge room once paid
		 * for (OpenChallengeBossDoor): the challenges all sit after the
		 * boss rooms in the list. */
		for (size_t ch = 0; ch < challengeToAdd.size(); ch++)
		{
			int entrance = challengeToAdd[ch].linkedRoom;
			if (entrance >= 0 && entrance < (int) rooms.size())
				rooms[entrance].challengeRoom = count + (int) bossToAdd.size() + (int) ch;
		}
	}

	// Builds the room's object list: the template's objects for its area
	// (LevelParser.StoreObj / StoreSwappedObj), mirrored if needed
	// (RoomObj.Reverse), filtered by the remaining doors
	// (AddRemoveExtraObjects), with procedural enemies resolved
	// (AddProceduralEnemies), then moved to castle coordinates.
	void LevelBuilder2::Materialize(PlacedRoom &room)
	{
		const RoomTemplate &r = T[room.templ];
		// Boss and challenge rooms only keep the objects meant for every area
		bool generic = room.name == "Boss" || room.name == "ChallengeBoss";
		LevelType area = room.levelType == LevelType::NONE ? LevelType::CASTLE : room.levelType;
		const float W = (float) room.width;
		std::vector<ObjData> objs;
		objs.reserve(r.objs.size() + room.extra.size());

		for (const ObjData &src : r.objs)
		{
			if (src.type == "DoorObj" || src.type == "PlayerStartObj") continue; // doors come from room.doors
			LevelType lt = (LevelType) src.levelType;
			ObjData o = src;
			if (generic)
			{
				if (lt != LevelType::NONE) continue;
			}
			else if (lt == LevelType::CASTLE && area != LevelType::CASTLE)
			{
				// StoreSwappedObj
				int idx = -1;
				for (int i = 0; i < ASSETSWAP_COUNT; i++)
					if (o.sprite == CASTLE_ASSETSWAP_LIST[i]) { idx = i; break; }
				if (idx < 0) continue;
				std::string swapped = SwapListName(area, idx);
				if (swapped.empty()) continue;
				size_t rp = swapped.find("RANDOM");
				if (rp != std::string::npos)
				{
					int numRandoms = swapped[rp + 6] - '0';
					int choice = RandomInt(1, numRandoms);
					swapped = swapped.substr(0, rp) + std::to_string(choice) + swapped.substr(rp + 7);
					if (swapped.find("TowerHole") != std::string::npos)
					{
						o.x += RandomInt(-50, 50);
						o.y += RandomInt(-50, 50);
						if (RandomInt(1, 100) > 70) continue; // 70% chance of the hole being visible
					}
					// The swapped GardenFloatingRock sprite becomes a
					// HoverObj in RoomRenderer::DecorationsFor.
				}
				if (swapped == "CastleAssetFrame_Sprite")
					swapped = "FramePicture" + std::to_string(RandomInt(1, 16)) + "_Sprite";
				if (swapped.find("GardenFairy") != std::string::npos)
				{
					o.x += RandomInt(-25, 25);
					o.y += RandomInt(-25, 25);
				}
				if (o.breakable && o.sprite.find("CastleAssetUrn") != std::string::npos) o.collidesTop = false;
				o.sprite = swapped;
			}
			else if (lt != LevelType::NONE && lt != area)
			{
				continue;
			}
			else if (lt == LevelType::CASTLE && o.sprite == "CastleAssetFrame_Sprite")
			{
				o.sprite = "FramePicture" + std::to_string(RandomInt(1, 16)) + "_Sprite";
			}
			objs.push_back(o);
		}

		if (room.reversed)
		{
			for (ObjData &o : objs)
			{
				ReverseName(o.name);
				if (o.type == "EnemyObj" || o.type == "EnemyOrbObj")
				{
					// Enemies (and orbs, which become enemies) mirror their position point
					if (o.type == "EnemyObj") o.flip = !o.flip;
					o.x = W - o.x;
					o.rotation = -o.rotation;
				}
				else if (IsRectObj(o))
				{
					if (o.type == "ChestObj") o.flip = !o.flip;
					MirrorRect(o, W);
					if (o.type == "BorderObj")
					{
						if (o.collidesRight) { if (!o.collidesLeft) o.collidesRight = false; o.collidesLeft = true; }
						else if (o.collidesLeft) { if (!o.collidesRight) o.collidesLeft = false; o.collidesRight = true; }
					}
				}
				else
				{
					o.flip = !o.flip;
					o.x = W - o.x;
					if (o.rotation != 0) o.rotation = -o.rotation;
				}
			}
		}

		// AddRemoveExtraObjects
		DoorFlags f = {};
		for (const Door &d : room.doors)
		{
			float dx = d.x - room.x, dy = d.y - room.y;
			switch (d.pos)
			{
			case DoorPos::Top:
				f.top = true;
				if (dx == 540) f.topLeft = true;
				if (room.width - dx == 780) f.topRight = true;
				break;
			case DoorPos::Bottom:
				f.bottom = true;
				if (dx == 540) f.bottomLeft = true;
				if (room.width - dx == 780) f.bottomRight = true;
				break;
			case DoorPos::Left:
				f.left = true;
				if (dy == 240) f.leftTop = true;
				if (room.height - dy == 480) f.leftBottom = true;
				break;
			case DoorPos::Right:
				f.right = true;
				if (dy == 240) f.rightTop = true;
				if (room.height - dy == 480) f.rightBottom = true;
				break;
			default:
				break;
			}
		}
		objs.erase(std::remove_if(objs.begin(), objs.end(), [&](const ObjData &o) { return RemovedByDoors(o.name, f); }), objs.end());

		// AddProceduralEnemies
		const uint8_t *pool = CASTLE_ENEMY_LIST, *diff = CASTLE_ENEMY_DIFFICULTY_LIST;
		int n = (int) sizeof(CASTLE_ENEMY_LIST);
		switch (room.levelType)
		{
		case LevelType::GARDEN: pool = GARDEN_ENEMY_LIST; diff = GARDEN_ENEMY_DIFFICULTY_LIST; n = (int) sizeof(GARDEN_ENEMY_LIST); break;
		case LevelType::TOWER: pool = TOWER_ENEMY_LIST; diff = TOWER_ENEMY_DIFFICULTY_LIST; n = (int) sizeof(TOWER_ENEMY_LIST); break;
		case LevelType::DUNGEON: pool = DUNGEON_ENEMY_LIST; diff = DUNGEON_ENEMY_DIFFICULTY_LIST; n = (int) sizeof(DUNGEON_ENEMY_LIST); break;
		default: break;
		}
		// One enemy kind per orb colour: red, blue, green, white, black,
		// each different from the previous one; yellow (expert) is any
		// kind but BouncySpike
		int colourType[6], colourDiff[6];
		int index = RandomInt(0, n - 1), stored = index;
		colourType[0] = pool[index]; colourDiff[0] = diff[index];
		for (int c = 1; c < 5; c++)
		{
			while (index == stored) index = RandomInt(0, n - 1);
			stored = index;
			colourType[c] = pool[index];
			colourDiff[c] = diff[index];
		}
		index = RandomInt(0, n - 1);
		colourType[5] = pool[index];
		while (colourType[5] == BouncySpike)
		{
			index = RandomInt(0, n - 1);
			colourType[5] = pool[index];
		}
		colourDiff[5] = 2; // EXPERT
		for (ObjData &o : objs)
		{
			if (o.type == "EnemyOrbObj" && !o.isWaypoint)
			{
				int c = (o.orbType >= 0 && o.orbType < 5) ? o.orbType : 5;
				while (o.forceFlying && IsWeighted(colourType[c]))
				{
					index = RandomInt(0, n - 1);
					colourType[c] = pool[index];
					if (c < 5) colourDiff[c] = diff[index];
				}
				o.type = "EnemyObj";
				o.enemyType = colourType[c];
				o.difficulty = DifficultyName(colourDiff[c]);
				o.procedural = true;
			}
			else if (o.type == "EnemyObj" && o.procedural)
			{
				// EnemyTagObj: any enemy of the area, basic difficulty
				o.enemyType = pool[RandomInt(0, n - 1)];
				o.difficulty = "BASIC";
			}
		}

		for (const ObjData &e : room.extra) objs.push_back(e);
		for (const Door &d : room.doors)
		{
			ObjData o;
			o.type = "DoorObj";
			o.x = d.x - room.x; o.y = d.y - room.y;
			o.width = (float) d.width; o.height = (float) d.height;
			o.doorPos = d.pos;
			o.bossDoor = d.bossDoor;
			objs.push_back(o);
		}
		for (ObjData &o : objs)
		{
			o.x += room.x;
			o.y += room.y;
		}
		room.objs = std::move(objs);
	}
}
