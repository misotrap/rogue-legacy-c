// Heirs: who you can pick after each death. Port of the rules in
// LineageObj.cs (constructor, CreateMaleName/CreateFemaleName,
// RandomizePortrait), ClassType.cs (GetRandomClass, Upgraded,
// GetSpellList), TraitType.cs (Rarity, CreateRandomTraits, TraitConflict)
// and LineageScreen.cs (NameCopies, CurrentBranchNameCopyFound).
// Pure logic: no drawing, no save files.
#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace Lineage
{
	// What the manor (skill tree) has unlocked; the game fills this from
	// SkillSystem (SkillType.*_Unlock / *_Up) and PlayerStats.
	struct Unlocks
	{
		bool ninja = false, banker = false, spellSword = false, lich = false;
		bool dragon = false;   // SkillType.SuperSecret
		bool traitor = false;  // ChallengeLastBossBeaten or GameConfig.UnlockTraitor == 2
		// Upgraded class (ClassType.Upgraded): indexed by the base class
		// (Knight..Lich, 0..7); an upgraded class is base + 8
		bool upgraded[8] = { false, false, false, false, false, false, false, false };
	};

	// PlayerLineageData (Types/LineageTypes.cs)
	struct Heir
	{
		std::string name;
		std::string romanNumeral; // "" or II, III... when the name was used before
		bool isFemale = false;
		uint8_t classType = 0;    // ClassType::
		uint8_t spell = 0;        // SpellType::
		uint8_t trait1 = 0, trait2 = 0; // TraitType:: (Traits.X, Traits.Y)
		uint8_t age = 30, childAge = 4;
		uint8_t headPiece = 1, chestPiece = 1, shoulderPiece = 1;

		bool HasTrait(uint8_t t) const { return trait1 == t || trait2 == t; }
	};

	class Generator
	{
	public:
		explicit Generator(uint32_t seed = 1);

		// Content/HeroNames.txt and HeroineNames.txt (copied to assets/ by
		// tools/convert_assets.py). Lines with "//" are comments.
		bool LoadNames(const char *maleFile, const char *femaleFile);

		// One new heir (LineageObj constructor + RandomizePortrait).
		// familyTree: names of every past ancestor (for roman numerals);
		// branch: heirs already offered this time (no duplicate names).
		Heir CreateHeir(const Unlocks &unlocks, const std::vector<std::string> &familyTree,
			const std::vector<Heir> &branch);

		// The three heirs offered after a death (LineageScreen.LoadCurrentBranches)
		std::vector<Heir> CreateBranches(const Unlocks &unlocks, const std::vector<std::string> &familyTree, int count = 3);

		uint8_t GetRandomClass(const Unlocks &unlocks);
		void CreateRandomTraits(uint8_t &trait1, uint8_t &trait2);
		static uint8_t Rarity(uint8_t trait);
		static bool TraitConflict(uint8_t trait1, uint8_t trait2);
		// Spells a class can start with (ClassType.GetSpellList); empty if none
		static std::vector<uint8_t> GetSpellList(uint8_t classType);
		static std::string ToRoman(int number);

		std::vector<std::string> maleNames, femaleNames;

	private:
		std::mt19937 rng;
		int RandomInt(int min, int max); // CDGMath.RandomInt: inclusive
		void CreateName(Heir &h, const std::vector<std::string> &familyTree, const std::vector<Heir> &branch);
	};
}
