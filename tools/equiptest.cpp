// Simulates chests, the blacksmith and the enchantress (no graphics):
//   make equiptest && ./equiptest [seed]
#include "../src/RogueCastle/Equipment/Equipment.h"

#include <cstdio>
#include <cstdlib>

using namespace Equipment;

int main(int argc, char **argv)
{
	std::mt19937 rng(argc > 1 ? (uint32_t) strtoul(argv[1], nullptr, 10) : 1);
	EquipmentSystem eq;
	static const char *const cat[] = { "sword", "helm", "chest", "limbs", "cape" };
	// Chests found while the castle gets harder: level 1..20, all colours
	for (int level = 1; level <= 20; level += 3)
		for (int chestType = 1; chestType <= 3; chestType++)
		{
			int c, i;
			if (eq.GiveRandomBlueprint(level, chestType, rng, c, i))
				printf("chest level %2d type %d: blueprint %-5s #%d (needs level %d, chest %d)\n", level, chestType, cat[c], i,
					EquipmentSystem::Item(c, i)->levelRequirement, EquipmentSystem::Item(c, i)->chestColourRequirement);
		}
	int gold = 5000;
	int maxWeight = EquipmentSystem::MaxWeight(50, 30, 0, 1, 5); // example values
	for (int c = 0; c < CATEGORIES; c++)
		for (int i = 0; i < BASE_TYPES; i++)
			if (eq.blueprint[c][i] == EquipmentState::FoundButNotSeen && eq.BuyBlueprint(c, i, gold))
			{
				bool ok = eq.Equip(c, i, maxWeight);
				printf("bought %-5s #%d for %d, equip %s (weight %d/%d)\n", cat[c], i, EquipmentSystem::Item(c, i)->cost,
					ok ? "ok" : "too heavy", eq.CurrentWeight(), maxWeight);
			}
	eq.rune[EquipmentCategoryType::Cape][EquipmentAbilityType::DoubleJump] = EquipmentState::FoundButNotSeen;
	eq.rune[EquipmentCategoryType::Helm][EquipmentAbilityType::Dash] = EquipmentState::FoundButNotSeen;
	gold += 1000;
	printf("rune costs: %d then ", eq.RuneCost(EquipmentCategoryType::Cape, EquipmentAbilityType::DoubleJump));
	eq.BuyRune(EquipmentCategoryType::Cape, EquipmentAbilityType::DoubleJump, gold);
	printf("%d\n", eq.RuneCost(EquipmentCategoryType::Helm, EquipmentAbilityType::Dash));
	eq.EquipRune(EquipmentCategoryType::Cape, EquipmentAbilityType::DoubleJump);
	Totals t = eq.ComputeTotals();
	printf("totals: weight %d, +%d hp, +%d dmg, +%d armor, +%d mp, +%d magic, crit %.2f, double jumps %d, %d gold left\n",
		t.weight, t.health, t.damage, t.armor, t.mana, t.magic, t.secondary[EquipmentSecondaryDataType::CritChance],
		t.runes[EquipmentAbilityType::DoubleJump], gold);
	return 0;
}
