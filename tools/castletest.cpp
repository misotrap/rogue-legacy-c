// Builds castles from assets/rooms.txt and checks them (no graphics):
//   make castletest && ./castletest [seed] [count] [map.ppm]
// Prints rooms per area, overlaps, open doors and enemy counts, and can
// draw a map (one pixel = 60 world units).
#include "../src/RogueCastle/Levels/LevelBuilder2.h"
#include "../src/RogueCastle/Levels/LevelParser.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Levels;

static bool Overlap(const PlacedRoom &a, const PlacedRoom &b)
{
	return b.x < a.x + a.width && a.x < b.x + b.width && b.y < a.y + a.height && a.y < b.y + b.height;
}

static void DrawMap(const Castle &c, const char *path)
{
	float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
	for (const PlacedRoom &r : c.rooms)
	{
		if (r.x < -50000 || r.y < -50000 || r.x > 50000 || r.y > 50000) continue; // boss rooms float away
		if (r.x < minX) minX = r.x;
		if (r.y < minY) minY = r.y;
		if (r.Right() > maxX) maxX = r.Right();
		if (r.Bottom() > maxY) maxY = r.Bottom();
	}
	int w = (int) ((maxX - minX) / 60) + 1, h = (int) ((maxY - minY) / 60) + 1;
	std::vector<unsigned char> px((size_t) w * h * 3, 20);
	auto fill = [&](float x, float y, float rw, float rh, unsigned char cr, unsigned char cg, unsigned char cb)
	{
		int x0 = (int) ((x - minX) / 60), y0 = (int) ((y - minY) / 60);
		int x1 = (int) ((x + rw - minX) / 60), y1 = (int) ((y + rh - minY) / 60);
		for (int j = y0; j < y1 && j < h; j++)
			for (int i = x0; i < x1 && i < w; i++)
				if (i >= 0 && j >= 0)
				{
					unsigned char *p = &px[((size_t) j * w + i) * 3];
					p[0] = cr; p[1] = cg; p[2] = cb;
				}
	};
	for (const PlacedRoom &r : c.rooms)
	{
		if (r.x < -50000 || r.y < -50000 || r.x > 50000 || r.y > 50000) continue;
		unsigned char cr = 90, cg = 90, cb = 110; // castle
		if (r.levelType == LevelType::GARDEN) { cr = 60; cg = 120; cb = 60; }
		else if (r.levelType == LevelType::TOWER) { cr = 70; cg = 90; cb = 160; }
		else if (r.levelType == LevelType::DUNGEON) { cr = 130; cg = 60; cb = 60; }
		else if (r.levelType == LevelType::NONE) { cr = 200; cg = 200; cb = 80; }
		fill(r.x, r.y, (float) r.width, (float) r.height, cr, cg, cb);
		for (const ObjData &o : r.objs)
			if (o.type == "CollHullObj" && o.rotation == 0) fill(o.x, o.y, o.width, o.height, 30, 30, 34);
		for (const Door &d : r.doors) fill(d.x, d.y, (float) d.width, (float) d.height, 240, 220, 120);
		if (r.name == "EntranceBoss") fill(r.x + 300, r.y + 200, 720, 300, 255, 80, 80);
		if (r.name == "Secret" || r.name == "Bonus") fill(r.x + 300, r.y + 200, 720, 300, 255, 255, 255);
	}
	FILE *f = fopen(path, "wb");
	if (f == nullptr) return;
	fprintf(f, "P6\n%d %d\n255\n", w, h);
	fwrite(px.data(), 1, px.size(), f);
	fclose(f);
	printf("map: %s (%dx%d)\n", path, w, h);
}

int main(int argc, char **argv)
{
	uint32_t seed = argc > 1 ? (uint32_t) strtoul(argv[1], nullptr, 10) : 1;
	int count = argc > 2 ? atoi(argv[2]) : 1;
	const char *mapPath = argc > 3 ? argv[3] : nullptr;

	auto t0 = std::chrono::steady_clock::now();
	std::vector<RoomTemplate> templates;
	if (!LoadRooms("assets/rooms.txt", templates))
	{
		fprintf(stderr, "cannot read assets/rooms.txt\n");
		return 1;
	}
	LevelBuilder2 builder(templates);
	auto t1 = std::chrono::steady_clock::now();
	printf("%zu room templates loaded in %.0f ms\n", templates.size(), std::chrono::duration<double, std::milli>(t1 - t0).count());

	int failures = 0;
	for (int n = 0; n < count; n++)
	{
		Castle castle;
		auto a = std::chrono::steady_clock::now();
		if (!builder.CreateLevel(castle, DefaultAreas(), 4, seed + n))
		{
			printf("seed %u: FAILED\n", seed + n);
			failures++;
			continue;
		}
		auto b = std::chrono::steady_clock::now();
		int perArea[5] = { 0 }, overlaps = 0, openDoors = 0, enemies = 0, objs = 0, bosses = 0, secrets = 0, bonus = 0;
		for (size_t i = 0; i < castle.rooms.size(); i++)
		{
			const PlacedRoom &r = castle.rooms[i];
			perArea[(int) r.levelType]++;
			if (r.name == "EntranceBoss") bosses++;
			if (r.name == "Secret") secrets++;
			if (r.name == "Bonus") bonus++;
			for (const Door &d : r.doors) if (!d.attached && !d.locked) openDoors++;
			for (const ObjData &o : r.objs)
			{
				objs++;
				if (o.type == "EnemyObj") enemies++;
			}
			for (size_t j = i + 1; j < castle.rooms.size(); j++)
				if (Overlap(r, castle.rooms[j])) overlaps++;
		}
		printf("seed %u: %zu rooms (castle %d, garden %d, tower %d, dungeon %d, other %d), %d boss entrances, "
			"%d secret, %d bonus, %d enemies, %d objects, %d overlaps, %d unattached doors, %.0f ms\n",
			seed + n, castle.rooms.size(), perArea[1], perArea[2], perArea[4], perArea[3], perArea[0], bosses,
			secrets, bonus, enemies, objs, overlaps, openDoors, std::chrono::duration<double, std::milli>(b - a).count());
		if (overlaps) failures++;
		if (mapPath != nullptr && n == 0) DrawMap(castle, mapPath);
	}
	return failures ? 1 : 0;
}
