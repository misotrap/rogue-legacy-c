// Renders the manor screen into PPM files (no window):
//   make manortest && ./manortest out -> out1.ppm (new game), out2.ppm (after purchases)
#include "../src/RogueCastle/Screens/ManorScreen.h"
#include "../src/Text/Loc.h"
#include "../src/platform.h"

#include <cstdio>
#include <string>

static uint32_t frame[SCREEN_W * SCREEN_H];

static void WritePpm(const std::string &path)
{
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

int main(int argc, char **argv)
{
	std::string out = argc > 1 ? argv[1] : "manor";
	Text::Loc::Load("assets", "fr");
	Save::GameState state;
	state.Gold = 3000;
	RogueCastle::ManorScreen screen;
	if (!screen.LoadContent("assets")) printf("some content missing\n");
	screen.Open(state);
	screen.Update(0, 0, 1.0 / 30);
	screen.Draw(frame);
	WritePpm(out + "1.ppm");
	// Buy the smithy, move up to Health Up, buy it twice
	screen.Update(BTN_B, BTN_B, 1.0 / 30);
	screen.Update(BTN_UP, BTN_UP, 1.0 / 30);
	screen.Update(BTN_B, BTN_B, 1.0 / 30);
	screen.Update(BTN_B, BTN_B, 1.0 / 30);
	screen.Draw(frame);
	WritePpm(out + "2.ppm");
	printf("gold %d, player level %d, selected skill %d\n", state.Gold, state.skills.PlayerLevel(), (int) screen.Selected());
	return 0;
}
