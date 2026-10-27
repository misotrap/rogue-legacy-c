// Renders the heir choice screen into PPM files (no window):
//   make lineagescreentest && ./lineagescreentest out
// Writes out1.ppm (first heir selected) and out2.ppm (after Right).
#include "../src/RogueCastle/Screens/LineageScreen.h"
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
	std::string out = argc > 1 ? argv[1] : "lineage";
	Text::Loc::Load("assets", "fr");
	Lineage::Generator gen(11);
	gen.LoadNames("assets/HeroNames.txt", "assets/HeroineNames.txt");
	Save::GameState state;
	int gold = 100000;
	state.skills.Buy(Skills::SkillType::Smithy, gold);
	state.familyTree.push_back(Save::FamilyTreeNode());
	state.familyTree.back().name = "Lancelot";

	RogueCastle::LineageScreen screen;
	if (!screen.LoadContent("assets")) printf("some content missing\n");
	screen.Open(state, gen);
	for (int i = 0; i < 10; i++) screen.Update(0, 0, 1.0 / 30);
	screen.Draw(frame);
	WritePpm(out + "1.ppm");
	screen.Update(BTN_RIGHT, BTN_RIGHT, 1.0 / 30);
	screen.Draw(frame);
	WritePpm(out + "2.ppm");
	bool chosen = screen.Update(BTN_B, BTN_B, 1.0 / 30);
	printf("chosen: %d, hero: %s (class %d)\n", chosen, state.hero.name.c_str(), state.hero.classType);
	return 0;
}
