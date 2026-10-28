#include "LevelParser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Levels
{
	DoorPos Opposite(DoorPos p)
	{
		switch (p)
		{
		case DoorPos::Left: return DoorPos::Right;
		case DoorPos::Right: return DoorPos::Left;
		case DoorPos::Top: return DoorPos::Bottom;
		case DoorPos::Bottom: return DoorPos::Top;
		default: return DoorPos::None;
		}
	}

	const char *DoorPosName(DoorPos p)
	{
		switch (p)
		{
		case DoorPos::Left: return "Left";
		case DoorPos::Right: return "Right";
		case DoorPos::Top: return "Top";
		case DoorPos::Bottom: return "Bottom";
		default: return "None";
		}
	}

	bool RoomTemplate::InPool(LevelType t) const
	{
		switch (t)
		{
		case LevelType::CASTLE: return castlePool;
		case LevelType::GARDEN: return gardenPool;
		case LevelType::TOWER: return towerPool;
		case LevelType::DUNGEON: return dungeonPool;
		default: return false;
		}
	}

	namespace
	{
		bool IsTrue(const char *v) { return v[0] == 'T' || v[0] == 't'; }

		DoorPos ParseDoorPos(const char *v)
		{
			if (!strcmp(v, "Left")) return DoorPos::Left;
			if (!strcmp(v, "Right")) return DoorPos::Right;
			if (!strcmp(v, "Top")) return DoorPos::Top;
			if (!strcmp(v, "Bottom")) return DoorPos::Bottom;
			return DoorPos::None;
		}

		// Splits a tab-separated record in place; fields[0] is the record kind
		int Split(char *line, char **fields, int max)
		{
			int n = 0;
			char *p = line;
			while (n < max)
			{
				fields[n++] = p;
				p = strchr(p, '\t');
				if (p == nullptr) break;
				*p++ = '\0';
			}
			return n;
		}

		void ApplyRoomAttr(RoomTemplate &r, const char *k, const char *v)
		{
			if (!strcmp(k, "Name")) r.name = v;
			else if (!strcmp(k, "Tag")) r.tag = v;
			else if (!strcmp(k, "X")) r.x = (float) atof(v);
			else if (!strcmp(k, "Y")) r.y = (float) atof(v);
			else if (!strcmp(k, "Width")) r.width = (int) atof(v);
			else if (!strcmp(k, "Height")) r.height = (int) atof(v);
			else if (!strcmp(k, "CastlePool")) r.castlePool = IsTrue(v);
			else if (!strcmp(k, "GardenPool")) r.gardenPool = IsTrue(v);
			else if (!strcmp(k, "TowerPool")) r.towerPool = IsTrue(v);
			else if (!strcmp(k, "DungeonPool")) r.dungeonPool = IsTrue(v);
		}

		void ApplyObjAttr(ObjData &o, const char *k, const char *v)
		{
			switch (k[0])
			{
			case 'B':
				if (!strcmp(k, "BossDoor")) o.bossDoor = IsTrue(v);
				else if (!strcmp(k, "BGLayer")) o.bgLayer = IsTrue(v);
				else if (!strcmp(k, "Breakable")) o.breakable = IsTrue(v);
				break;
			case 'C':
				if (!strcmp(k, "Collidable")) o.collidable = IsTrue(v);
				else if (!strcmp(k, "CollidesTop")) o.collidesTop = IsTrue(v);
				else if (!strcmp(k, "CollidesBottom")) o.collidesBottom = IsTrue(v);
				else if (!strcmp(k, "CollidesLeft")) o.collidesLeft = IsTrue(v);
				else if (!strcmp(k, "CollidesRight")) o.collidesRight = IsTrue(v);
				break;
			case 'D':
				if (!strcmp(k, "DoorPos")) o.doorPos = ParseDoorPos(v);
				else if (!strcmp(k, "Difficulty")) o.difficulty = v;
				break;
			case 'E':
				if (!strcmp(k, "EnemyType")) o.enemyType = atoi(v);
				break;
			case 'F':
				if (!strcmp(k, "Flip")) o.flip = IsTrue(v);
				else if (!strcmp(k, "ForceFlying")) o.forceFlying = IsTrue(v);
				else if (!strcmp(k, "Fairy")) o.fairy = IsTrue(v);
				break;
			case 'H':
				if (!strcmp(k, "Height")) o.height = (float) atof(v);
				break;
			case 'I':
				if (!strcmp(k, "IsWaypoint")) o.isWaypoint = IsTrue(v);
				else if (!strcmp(k, "InitialDelay")) o.initialDelay = (float) atof(v);
				break;
			case 'L':
				if (!strcmp(k, "LevelType")) o.levelType = atoi(v);
				break;
			case 'N':
				if (!strcmp(k, "Name")) o.name = v;
				break;
			case 'O':
				if (!strcmp(k, "OrbType")) o.orbType = atoi(v);
				break;
			case 'P':
				if (!strcmp(k, "Procedural")) o.procedural = IsTrue(v);
				break;
			case 'R':
				if (!strcmp(k, "Rotation")) o.rotation = (float) atof(v);
				break;
			case 'S':
				if (!strcmp(k, "SpriteName")) o.sprite = v;
				else if (!strcmp(k, "ScaleX")) o.scaleX = (float) atof(v);
				else if (!strcmp(k, "ScaleY")) o.scaleY = (float) atof(v);
				break;
			case 'T':
				if (!strcmp(k, "Tag")) o.tag = v;
				break;
			case 'W':
				if (!strcmp(k, "Width")) o.width = (float) atof(v);
				else if (!strcmp(k, "Weighted")) o.weighted = IsTrue(v);
				break;
			case 'X':
				if (!k[1]) o.x = (float) atof(v);
				break;
			case 'Y':
				if (!k[1]) o.y = (float) atof(v);
				break;
			}
		}
	}

	bool LoadRooms(const char *path, std::vector<RoomTemplate> &out)
	{
		FILE *f = fopen(path, "r");
		if (f == nullptr) return false;
		static char line[8192];
		char *fields[64];
		std::string mapFile;
		RoomTemplate *room = nullptr;
		while (fgets(line, sizeof(line), f))
		{
			size_t len = strlen(line);
			while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
			int n = Split(line, fields, 64);
			if (!strcmp(fields[0], "M") && n > 1)
			{
				mapFile = fields[1];
			}
			else if (!strcmp(fields[0], "ROOM"))
			{
				out.emplace_back();
				room = &out.back();
				room->mapFile = mapFile;
				room->isDLCMap = mapFile.find("DLC") != std::string::npos;
				for (int i = 1; i < n; i++)
				{
					char *eq = strchr(fields[i], '=');
					if (eq == nullptr) continue;
					*eq = '\0';
					ApplyRoomAttr(*room, fields[i], eq + 1);
				}
			}
			else if (!strcmp(fields[0], "G") && room != nullptr && n > 1)
			{
				ObjData o;
				o.type = fields[1];
				for (int i = 2; i < n; i++)
				{
					char *eq = strchr(fields[i], '=');
					if (eq == nullptr) continue;
					*eq = '\0';
					ApplyObjAttr(o, fields[i], eq + 1);
				}
				// Room-relative coordinates: rooms move around when placed
				o.x -= room->x;
				o.y -= room->y;
				room->objs.push_back(o);
			}
		}
		fclose(f);
		return true;
	}
}
