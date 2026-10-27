// Prints heirs as the lineage screen would offer them (no graphics):
//   make lineagetest && ./lineagetest [seed] [generations]
#include "../src/RogueCastle/Lineage/Lineage.h"
#include "../src/RogueCastle/Types/ClassType.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
	uint32_t seed = argc > 1 ? (uint32_t) strtoul(argv[1], nullptr, 10) : 1;
	int generations = argc > 2 ? atoi(argv[2]) : 5;
	Lineage::Generator gen(seed);
	if (!gen.LoadNames("assets/HeroNames.txt", "assets/HeroineNames.txt"))
	{
		fprintf(stderr, "cannot read assets/HeroNames.txt / HeroineNames.txt\n");
		return 1;
	}
	Lineage::Unlocks unlocks;
	std::vector<std::string> familyTree;
	int traitCount[3] = { 0, 0, 0 };
	for (int g = 0; g < generations; g++)
	{
		// Unlock more as the family goes on, like buying manor upgrades
		if (g == 2) { unlocks.ninja = unlocks.banker = true; unlocks.upgraded[ClassType::Knight] = true; }
		if (g == 4) { unlocks.lich = unlocks.spellSword = true; unlocks.dragon = true; }
		std::vector<Lineage::Heir> branch = gen.CreateBranches(unlocks, familyTree);
		printf("generation %d:\n", g + 1);
		for (const Lineage::Heir &h : branch)
		{
			int n = (h.trait1 != 0) + (h.trait2 != 0);
			traitCount[n]++;
			printf("  %-10s %-4s %s  class %2d  spell %2d  traits %2d %2d  age %d  look %d/%d/%d\n", h.name.c_str(),
				h.romanNumeral.c_str(), h.isFemale ? "F" : "M", h.classType, h.spell, h.trait1, h.trait2, h.age,
				h.headPiece, h.chestPiece, h.shoulderPiece);
		}
		familyTree.push_back(branch[0].name); // the first heir is chosen and dies
	}
	printf("heirs with 0/1/2 traits: %d/%d/%d\n", traitCount[0], traitCount[1], traitCount[2]);
	return 0;
}
