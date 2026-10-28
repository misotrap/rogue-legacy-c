// Renders rooms of a generated castle with RoomRenderer into PPM files:
//   make roomrendertest && ./roomrendertest out [seed]
// Writes out-castle.ppm, out-tower.ppm, out-dungeon.ppm, out-garden.ppm.
#include "../src/RogueCastle/Levels/LevelBuilder2.h"
#include "../src/RogueCastle/Levels/LevelParser.h"
#include "../src/RogueCastle/Rendering/RoomRenderer.h"
#include "../src/SpriteSystem/SpriteLibrary.h"
#include "../src/platform.h"

#include <cstdio>
#include <cstdlib>
#include <string>

static uint32_t frame[SCREEN_W * SCREEN_H];

int main(int argc, char **argv)
{
	std::string out = argc > 1 ? argv[1] : "room";
	uint32_t seed = argc > 2 ? (uint32_t) atoi(argv[2]) : 3;
	std::vector<Levels::RoomTemplate> templates;
	Levels::LoadRooms("assets/rooms.txt", templates);
	Levels::LevelBuilder2 builder(templates);
	Levels::Castle castle;
	builder.CreateLevel(castle, Levels::DefaultAreas(), 4, seed);
	RogueCastle::RoomRenderer renderer;
	if (!renderer.LoadContent("assets")) printf("some sheets missing\n");
	SpriteSystem::SpriteLibrary::LoadSheet("assets", "playerSpritesheetBig");
	const char *names[] = { "", "castle", "garden", "dungeon", "tower" };
	bool done[5] = {};
	for (const Levels::PlacedRoom &room : castle.rooms)
	{
		int t = (int) room.levelType;
		if (t == 0 || done[t] || room.name != "Room_Obj 1") continue;
		done[t] = true;
		DS2DEngine::Camera2D camera;
		camera.Zoom(0.5f);
		camera.Position(Vector2(room.x + 660, room.y + 360));
		gfx_begin(frame);
		gfx_clear(0xFF000000u);
		renderer.Draw(&camera, room);
		std::string path = out + "-" + names[t] + ".ppm";
		FILE *f = fopen(path.c_str(), "wb");
		fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
		for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
		{
			unsigned char rgb[3] = { (unsigned char) (frame[i] >> 16), (unsigned char) (frame[i] >> 8), (unsigned char) frame[i] };
			fwrite(rgb, 1, 3, f);
		}
		fclose(f);
		printf("wrote %s\n", path.c_str());
	}
	return 0;
}
