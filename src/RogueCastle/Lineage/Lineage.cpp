#include "Lineage.h"

#include "../Types/ClassType.h"
#include "../Types/PlayerPart.h"
#include "../Types/SpellType.h"
#include "../Types/TraitType.h"

#include <cstdio>
#include <cstring>

namespace Lineage
{
	Generator::Generator(uint32_t seed) : rng(seed)
	{
	}

	int Generator::RandomInt(int min, int max)
	{
		if (max < min) return min;
		return std::uniform_int_distribution<int>(min, max)(rng);
	}

	static bool LoadNameFile(const char *path, std::vector<std::string> &out)
	{
		FILE *f = fopen(path, "r");
		if (f == nullptr) return false;
		char line[256];
		while (fgets(line, sizeof(line), f))
		{
			size_t n = strlen(line);
			while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r' || line[n - 1] == ' ')) line[--n] = '\0';
			// Skip a UTF-8 byte order mark
			const char *s = line;
			if ((unsigned char) s[0] == 0xEF && (unsigned char) s[1] == 0xBB && (unsigned char) s[2] == 0xBF) s += 3;
			if (s[0] == '\0' || strstr(s, "//") != nullptr) continue;
			out.push_back(s);
		}
		fclose(f);
		return true;
	}

	bool Generator::LoadNames(const char *maleFile, const char *femaleFile)
	{
		maleNames.clear();
		femaleNames.clear();
		bool ok = LoadNameFile(maleFile, maleNames) && LoadNameFile(femaleFile, femaleNames);
		return ok && !maleNames.empty() && !femaleNames.empty();
	}

	std::string Generator::ToRoman(int number)
	{
		static const int values[] = { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };
		static const char *const numerals[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
		std::string out;
		for (int i = 0; i < 13; i++)
			while (number >= values[i])
			{
				out += numerals[i];
				number -= values[i];
			}
		return out;
	}

	// LineageObj.CreateMaleName / CreateFemaleName
	void Generator::CreateName(Heir &h, const std::vector<std::string> &familyTree, const std::vector<Heir> &branch)
	{
		const std::vector<std::string> &names = h.isFemale ? femaleNames : maleNames;
		if (names.empty())
		{
			h.name = h.isFemale ? "Heroine" : "Hero";
			h.romanNumeral = "";
			return;
		}
		auto inBranch = [&](const std::string &n)
		{
			// RevisionNumber > 0 rule: exact match
			for (const Heir &b : branch)
				if (b.name == n) return true;
			return false;
		};
		std::string name = names[RandomInt(0, (int) names.size() - 1)];
		int countBreaker = 0;
		while (inBranch(name))
		{
			name = names[RandomInt(0, (int) names.size() - 1)];
			if (++countBreaker > 20) break;
		}
		if (name.size() > 10) name = name.substr(0, 9) + ".";
		// LineageScreen.NameCopies: ancestors whose name contains this one
		int copies = 0;
		for (const std::string &n : familyTree)
			if (n.find(name) != std::string::npos) copies++;
		h.name = name;
		h.romanNumeral = copies > 0 ? ToRoman(copies + 1) : "";
	}

	uint8_t Generator::Rarity(uint8_t t)
	{
		using namespace TraitType;
		switch (t)
		{
		case Gay: case Gigantism: case Dwarfism: case Baldness: case Endomorph: case Tourettes: case Hyperactive:
		case OCD: case Hypergonadism: case StereoBlind:
			return 1;
		case ColorBlind: case NearSighted: case Ectomorph: case Dextrocardia: case IBS: case TunnelVision:
		case Ambilevous: case PAD: case Alektorophobia: case Hypermobility: case EideticMemory: case Savant:
		case NoFurniture: case PlatformsOpen: case Glaucoma: case Clonus: case Prosopagnosia:
			return 2;
		case FarSighted: case Dyslexia: case Alzheimers: case Hypogonadism: case Vertigo: case Hypochondriac:
		case Dementia: case Nostalgic: case CIP: case TheOne:
			return 3;
		default:
			return 0;
		}
	}

	bool Generator::TraitConflict(uint8_t x, uint8_t y)
	{
		using namespace TraitType;
		auto pair = [&](uint8_t a, uint8_t b) { return (x == a && y == b) || (x == b && y == a); };
		return pair(Hypergonadism, Hypogonadism) || pair(Endomorph, Ectomorph) || pair(Gigantism, Dwarfism)
			|| pair(NearSighted, FarSighted) || pair(ColorBlind, Nostalgic);
	}

	void Generator::CreateRandomTraits(uint8_t &trait1, uint8_t &trait2)
	{
		trait1 = trait2 = 0;
		int numTraits = 0;
		const int maxTraits = 2;
		int baseChanceForTrait = 94;     // chance of at least one trait
		const int dropRateChanceForTrait = 39; // less for each further trait
		const int minChanceForTrait = 1;
		int traitChance = RandomInt(0, 100);
		for (int i = 0; i < maxTraits; i++)
		{
			if (traitChance < baseChanceForTrait) numTraits++;
			baseChanceForTrait -= dropRateChanceForTrait;
			if (baseChanceForTrait < minChanceForTrait) baseChanceForTrait = minChanceForTrait;
		}
		static const int rarityParams[] = { 48, 37, 15 }; // odds of rarity 1, 2, 3

		auto pickRarityList = [&](std::vector<uint8_t> &list)
		{
			int totalChance = 0, chance = RandomInt(0, 100);
			uint8_t rarity = 0;
			for (int k = 0; k < 3; k++)
			{
				totalChance += rarityParams[k];
				if (chance <= totalChance)
				{
					rarity = (uint8_t) (k + 1);
					break;
				}
			}
			list.clear();
			for (uint8_t i = 0; i < TraitType::Total; i++)
				if (Rarity(i) == rarity) list.push_back(i);
		};

		std::vector<uint8_t> traitList;
		if (numTraits > 0)
		{
			pickRarityList(traitList);
			if (!traitList.empty()) trait1 = traitList[RandomInt(0, (int) traitList.size() - 1)];
		}
		if (numTraits > 1)
		{
			pickRarityList(traitList);
			if (traitList.empty()) return;
			int guard = 0;
			do
			{
				trait2 = traitList[RandomInt(0, (int) traitList.size() - 1)];
			} while ((trait2 == trait1 || TraitConflict(trait1, trait2)) && ++guard < 1000);
		}
	}

	uint8_t Generator::GetRandomClass(const Unlocks &u)
	{
		std::vector<uint8_t> list = { ClassType::Knight, ClassType::Wizard, ClassType::Barbarian, ClassType::Assassin };
		if (u.ninja) list.push_back(ClassType::Ninja);
		if (u.banker) list.push_back(ClassType::Banker);
		if (u.spellSword) list.push_back(ClassType::SpellSword);
		if (u.lich) list.push_back(ClassType::Lich);
		if (u.dragon) list.push_back(ClassType::Dragon);
		if (u.traitor) list.push_back(ClassType::Traitor);
		uint8_t c = list[RandomInt(0, (int) list.size() - 1)];
		if (c < 8 && u.upgraded[c]) c = (uint8_t) (c + 8); // ClassType.Upgraded
		return c;
	}

	std::vector<uint8_t> Generator::GetSpellList(uint8_t classType)
	{
		using namespace SpellType;
		switch (classType)
		{
		case ClassType::Knight: case ClassType::Knight2:
			return { Axe, Dagger, Boomerang, DualBlades, Close, Bounce };
		case ClassType::Barbarian: case ClassType::Barbarian2:
			return { Axe, Dagger, Boomerang, DualBlades, Close };
		case ClassType::Assassin: case ClassType::Assassin2:
			return { Axe, Dagger, Translocator, Boomerang, DualBlades, Bounce };
		case ClassType::Banker: case ClassType::Banker2:
			return { Axe, Dagger, Boomerang, DualBlades, DamageShield, Bounce };
		case ClassType::Lich: case ClassType::Lich2:
			return { Nuke, DamageShield, Bounce };
		case ClassType::SpellSword: case ClassType::SpellSword2:
			return { Axe, Dagger, Boomerang, DualBlades, Close, DamageShield };
		case ClassType::Wizard: case ClassType::Wizard2:
			return { Axe, Dagger, TimeStop, Boomerang, DualBlades, Close, DamageShield, Bounce };
		case ClassType::Ninja: case ClassType::Ninja2:
			return { Axe, Dagger, Translocator, Boomerang, DualBlades, Close, Bounce };
		case ClassType::Dragon:
			return { DragonFire };
		case ClassType::Traitor:
			return { RapidDagger };
		default:
			return {};
		}
	}

	Heir Generator::CreateHeir(const Unlocks &unlocks, const std::vector<std::string> &familyTree, const std::vector<Heir> &branch)
	{
		Heir h;
		h.isFemale = RandomInt(0, 1) > 0;
		CreateName(h, familyTree, branch);
		CreateRandomTraits(h.trait1, h.trait2);
		h.classType = GetRandomClass(unlocks);

		// A lich cannot have Dextrocardia (it swaps health and mana)
		while ((h.classType == ClassType::Lich || h.classType == ClassType::Lich2) && h.HasTrait(TraitType::Dextrocardia))
			CreateRandomTraits(h.trait1, h.trait2);
		// Savant rerolls spells: pointless for the classes with fixed spell sets
		while ((h.classType == ClassType::Wizard || h.classType == ClassType::Wizard2 || h.classType == ClassType::Dragon)
			&& h.HasTrait(TraitType::Savant))
			CreateRandomTraits(h.trait1, h.trait2);

		std::vector<uint8_t> spells = GetSpellList(h.classType);
		if (!spells.empty())
		{
			int guard = 0;
			do
			{
				h.spell = spells[RandomInt(0, (int) spells.size() - 1)];
			} while ((h.spell == SpellType::DamageShield || h.spell == SpellType::TimeStop || h.spell == SpellType::Translocator)
				&& h.HasTrait(TraitType::Savant) && ++guard < 1000);
		}
		h.age = (uint8_t) RandomInt(18, 30);
		h.childAge = (uint8_t) RandomInt(2, 5);

		// RandomizePortrait
		int head = RandomInt(1, PlayerPart::NumHeadPieces);
		int shoulder = RandomInt(1, PlayerPart::NumShoulderPieces);
		int chest = RandomInt(1, PlayerPart::NumChestPieces);
		if (h.classType == ClassType::Traitor) head = PlayerPart::IntroHelm;
		else if (h.classType == ClassType::Dragon) head = PlayerPart::DragonHelm;
		h.headPiece = (uint8_t) head;
		h.shoulderPiece = (uint8_t) shoulder;
		h.chestPiece = (uint8_t) chest;
		return h;
	}

	std::vector<Heir> Generator::CreateBranches(const Unlocks &unlocks, const std::vector<std::string> &familyTree, int count)
	{
		std::vector<Heir> branch;
		for (int i = 0; i < count; i++)
			branch.push_back(CreateHeir(unlocks, familyTree, branch));
		return branch;
	}
}
