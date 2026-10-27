// Renders the blacksmith and enchantress screens into PPM files (no window):
//   make equipscreentest && ./equipscreentest out -> out-smith.ppm out-enchant.ppm
#include "../src/RogueCastle/Screens/EquipmentScreen.h"
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
	std::string out = argc > 1 ? argv[1] : "equip";
	Text::Loc::Load("assets", "fr");
	Save::GameState state;
	state.Gold = 4000;
	// A few found blueprints and runes
	for (int i : { 0, 1, 5, 2, 7 }) state.equipment.blueprint[0][i] = EquipmentState::FoundButNotSeen;
	state.equipment.blueprint[1][0] = EquipmentState::FoundButNotSeen;
	state.equipment.rune[4][0] = EquipmentState::FoundButNotSeen;
	state.equipment.rune[4][3] = EquipmentState::FoundButNotSeen;

	RogueCastle::EquipmentScreen smith(RogueCastle::EquipmentScreen::Mode::Blacksmith);
	if (!smith.LoadContent("assets")) printf("some content missing\n");
	smith.Open(state, 100);
	smith.Update(BTN_B, BTN_B, 1.0 / 30);   // buy sword #0
	smith.Update(BTN_B, BTN_B, 1.0 / 30);   // equip it
	smith.Update(BTN_RIGHT, BTN_RIGHT, 1.0 / 30);
	smith.Update(BTN_B, BTN_B, 1.0 / 30);   // buy sword #1
	smith.Update(BTN_RIGHT, BTN_RIGHT, 1.0 / 30); // look at sword #2
	smith.Draw(frame);
	WritePpm(out + "-smith.ppm");

	RogueCastle::EquipmentScreen enchant(RogueCastle::EquipmentScreen::Mode::Enchantress);
	enchant.LoadContent("assets");
	enchant.Open(state, 100);
	enchant.Update(BTN_L1, BTN_L1, 1.0 / 30); // to the cape
	enchant.Update(BTN_B, BTN_B, 1.0 / 30);   // buy the double jump rune
	enchant.Update(BTN_B, BTN_B, 1.0 / 30);   // equip it
	enchant.Update(BTN_RIGHT, BTN_RIGHT, 1.0 / 30);
	enchant.Draw(frame);
	WritePpm(out + "-enchant.ppm");
	printf("gold %d, sword equipped %d, cape rune %d\n", state.Gold, state.equipment.equipped[0], state.equipment.equippedRune[4]);
	return 0;
}
