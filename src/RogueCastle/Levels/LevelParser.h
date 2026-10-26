// Reads assets/rooms.txt (tools/convert_assets.py) into room templates.
// Port of LevelParser.cs, minus the per-area copies (see RoomData.h).
#pragma once

#include "RoomData.h"

namespace Levels
{
	// Returns false if the file cannot be read. Every <RoomObject> becomes
	// one template, in file order; LevelBuilder2::Index sorts them into pools.
	bool LoadRooms(const char *path, std::vector<RoomTemplate> &out);
}
