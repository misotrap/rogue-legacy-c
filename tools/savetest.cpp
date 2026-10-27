// Save system check (no graphics): plays a fake run, saves, reloads and
// compares; then checks that a truncated save falls back to the backup.
//   make savetest && ./savetest
#include "../src/RogueCastle/Save/SaveGame.h"

#include <cstdio>
#include <cstring>

using namespace Save;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAILED: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

int main()
{
	const char *path = "/tmp/rlc-savetest.sav";
	remove(path);
	std::string bak = std::string(path) + ".bak";
	remove(bak.c_str());

	GameState a;
	Lineage::Generator gen(7);
	gen.LoadNames("assets/HeroNames.txt", "assets/HeroineNames.txt");
	a.currentBranches = gen.CreateBranches(a.skills.ClassUnlocks(), a.FamilyTreeNames());
	a.hero = a.currentBranches[1];
	a.Gold = 1234;
	a.TutorialComplete = true;
	a.EyeballBossBeaten = true;
	a.LichHealthMod = 0.75f;
	a.TotalHoursPlayed = 3.5f;
	FamilyTreeNode dead;
	dead.name = "Sir|Lancelot"; // '|' must not break the file
	dead.romanNumeral = "II";
	dead.classType = 8;
	dead.trait1 = 3;
	dead.beatenABoss = true;
	a.familyTree.push_back(dead);
	a.enemiesKilled[15][0] = 42; // skeletons
	a.castleExists = true;
	a.castleSeed = 987654;
	a.roomsVisited = { 1, 1, 0, 1 };
	int gold = 5000;
	a.skills.Buy(Skills::SkillType::Smithy, gold);
	a.skills.Buy(Skills::SkillType::Health_Up, gold);
	a.equipment.blueprint[0][0] = EquipmentState::FoundButNotSeen;
	a.equipment.BuyBlueprint(0, 0, gold);
	a.equipment.Equip(0, 0, 100);
	a.equipment.rune[4][0] = EquipmentState::FoundButNotSeen;
	a.equipment.BuyRune(4, 0, gold);
	a.equipment.EquipRune(4, 0);

	CHECK(Write(a, path));
	CHECK(Write(a, path)); // second save makes a .bak
	GameState b;
	CHECK(Read(b, path));
	CHECK(b.Gold == 1234);
	CHECK(b.TutorialComplete && b.EyeballBossBeaten && !b.FairyBossBeaten);
	CHECK(b.LichHealthMod == 0.75f);
	CHECK(b.hero.name == a.hero.name && b.hero.classType == a.hero.classType && b.hero.spell == a.hero.spell);
	CHECK(b.currentBranches.size() == 3);
	CHECK(b.familyTree.size() == 1 && b.familyTree[0].name == "Sir Lancelot" && b.familyTree[0].classType == 8 && b.familyTree[0].beatenABoss);
	CHECK(b.enemiesKilled[15][0] == 42);
	CHECK(b.castleExists && b.castleSeed == 987654 && b.roomsVisited.size() == 4 && b.roomsVisited[2] == 0);
	CHECK(b.skills.Level(Skills::SkillType::Health_Up) == 1 && b.skills.PlayerLevel() == 2 && b.skills.Visible(Skills::SkillType::Mana_Up));
	CHECK(b.equipment.equipped[0] == 0 && b.equipment.equippedRune[4] == 0 && b.equipment.totalRunesPurchased == 1);

	// Power cut while saving: the main file is cut short -> use the .bak
	FILE *f = fopen(path, "w");
	fputs("RogueLegacyC-save 1\nGold 1\n", f);
	fclose(f);
	GameState c;
	CHECK(Read(c, path));
	CHECK(c.Gold == 1234);

	// No save at all: a new game
	remove(path);
	remove(bak.c_str());
	GameState d;
	CHECK(!Read(d, path));
	CHECK(d.Gold == 0 && d.skills.Visible(Skills::SkillType::Smithy));

	printf(failures ? "%d check(s) failed\n" : "all save checks passed\n", failures);
	return failures ? 1 : 0;
}
