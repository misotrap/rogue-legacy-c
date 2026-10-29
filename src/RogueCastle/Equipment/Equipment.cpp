#include "Equipment.h"

#include <cstring>
#include <vector>

namespace Equipment
{
	EquipmentSystem::EquipmentSystem()
	{
		Reset();
	}

	void EquipmentSystem::Reset()
	{
		memset(blueprint, EquipmentState::NotFound, sizeof(blueprint));
		memset(rune, EquipmentState::NotFound, sizeof(rune));
		memset(equipped, -1, sizeof(equipped));
		memset(equippedRune, -1, sizeof(equippedRune));
		totalRunesPurchased = 0;
		/* PlayerStats.cs line 213: a new legacy starts with the bronze helm,
		 * limbs and sword blueprints and two runes found */
		blueprint[EquipmentCategoryType::Helm][EquipmentBaseType::Bronze] = EquipmentState::FoundButNotSeen;
		blueprint[EquipmentCategoryType::Limbs][EquipmentBaseType::Bronze] = EquipmentState::FoundButNotSeen;
		blueprint[EquipmentCategoryType::Sword][EquipmentBaseType::Bronze] = EquipmentState::FoundButNotSeen;
		rune[EquipmentCategoryType::Helm][EquipmentAbilityType::DoubleJump] = EquipmentState::FoundButNotSeen;
		rune[EquipmentCategoryType::Sword][EquipmentAbilityType::Dash] = EquipmentState::FoundButNotSeen;
	}

	const ItemData *EquipmentSystem::Item(int category, int index)
	{
		for (int i = 0; i < ITEM_COUNT; i++)
			if (ITEMS[i].category == category && ITEMS[i].index == index) return &ITEMS[i];
		return nullptr;
	}

	static bool Found(uint8_t state)
	{
		return state == EquipmentState::FoundButNotSeen || state == EquipmentState::FoundAndSeen;
	}

	static bool Owned(uint8_t state)
	{
		return state == EquipmentState::Purchased;
	}

	bool EquipmentSystem::BuyBlueprint(int category, int index, int &gold)
	{
		const ItemData *item = Item(category, index);
		if (item == nullptr || !Found(blueprint[category][index]) || gold < item->cost) return false;
		gold -= item->cost;
		blueprint[category][index] = EquipmentState::Purchased;
		return true;
	}

	bool EquipmentSystem::Equip(int category, int index, int maxWeight)
	{
		const ItemData *item = Item(category, index);
		if (item == nullptr || !Owned(blueprint[category][index])) return false;
		if (equipped[category] == index)
		{
			equipped[category] = -1; // taking it off
			return true;
		}
		// BlacksmithScreen: new weight = item + current - the piece it replaces
		int weightReduction = 0;
		if (equipped[category] >= 0)
		{
			const ItemData *current = Item(category, equipped[category]);
			if (current != nullptr) weightReduction = current->weight;
		}
		if (item->weight + CurrentWeight() - weightReduction > maxWeight) return false; // too heavy
		// What is worn lives in 'equipped' (PlayerStats.GetEquippedArray);
		// the blueprint itself stays Purchased
		equipped[category] = (int8_t) index;
		return true;
	}

	int EquipmentSystem::RuneCost(int category, int ability) const
	{
		int base = 9999; // not sold (EquipmentSystem constructor default)
		for (int i = 0; i < RUNE_COST_COUNT; i++)
			if (RUNE_COSTS[i].category == category && RUNE_COSTS[i].ability == ability) base = RUNE_COSTS[i].cost;
		return base + totalRunesPurchased * ABILITY_COST_MOD;
	}

	bool EquipmentSystem::BuyRune(int category, int ability, int &gold)
	{
		if (category < 0 || category >= CATEGORIES || ability < 0 || ability >= ABILITIES) return false;
		int cost = RuneCost(category, ability);
		if (!Found(rune[category][ability]) || gold < cost) return false;
		gold -= cost;
		rune[category][ability] = EquipmentState::Purchased;
		totalRunesPurchased++;
		return true;
	}

	bool EquipmentSystem::EquipRune(int category, int ability)
	{
		if (category < 0 || category >= CATEGORIES || ability < 0 || ability >= ABILITIES) return false;
		if (!Owned(rune[category][ability])) return false;
		if (equippedRune[category] == ability)
		{
			equippedRune[category] = -1;
			return true;
		}
		equippedRune[category] = (int8_t) ability;
		return true;
	}

	Totals EquipmentSystem::ComputeTotals() const
	{
		Totals t;
		for (int c = 0; c < CATEGORIES; c++)
		{
			if (equipped[c] >= 0)
			{
				const ItemData *item = Item(c, equipped[c]);
				if (item != nullptr)
				{
					t.weight += item->weight;
					t.health += item->bonusHealth;
					t.damage += item->bonusDamage;
					t.armor += item->bonusArmor;
					t.mana += item->bonusMana;
					t.magic += item->bonusMagic;
					for (int s = 0; s < item->numSecondary; s++)
					{
						int type = item->secondary[s].type;
						if (type >= 0 && type < EquipmentSecondaryDataType::Total) t.secondary[type] += item->secondary[s].amount;
					}
				}
			}
			if (equippedRune[c] >= 0) t.runes[equippedRune[c]]++;
		}
		return t;
	}

	int EquipmentSystem::MaxWeight(int baseWeight, float equipUpModifier, float equipUpFinalModifier, int bonusWeightDrops, int itemStatWeightAmount)
	{
		return (int) (baseWeight + equipUpModifier + equipUpFinalModifier) + bonusWeightDrops * itemStatWeightAmount;
	}

	int EquipmentSystem::TotalBlueprintsFound() const
	{
		int n = 0;
		for (int c = 0; c < CATEGORIES; c++)
			for (int i = 0; i < BASE_TYPES; i++)
				if (blueprint[c][i] != EquipmentState::NotFound) n++;
		return n;
	}

	bool EquipmentSystem::GiveRandomBlueprint(int chestLevel, int chestType, std::mt19937 &rng, int &category, int &index)
	{
		if (TotalBlueprintsFound() >= CATEGORIES * BASE_TYPES) return false;
		std::vector<std::pair<int, int>> possible;
		for (int c = 0; c < CATEGORIES; c++)
			for (int i = 0; i < BASE_TYPES; i++)
			{
				if (blueprint[c][i] != EquipmentState::NotFound) continue;
				const ItemData *item = Item(c, i);
				if (item != nullptr && chestLevel >= item->levelRequirement && chestType >= item->chestColourRequirement)
					possible.push_back({ c, i });
			}
		if (possible.empty()) return false;
		const auto &chosen = possible[std::uniform_int_distribution<int>(0, (int) possible.size() - 1)(rng)];
		category = chosen.first;
		index = chosen.second;
		blueprint[category][index] = EquipmentState::FoundButNotSeen;
		return true;
	}

	int EquipmentSystem::TotalRunesFound() const
	{
		int n = 0;
		for (int c = 0; c < CATEGORIES; c++)
			for (int a = 0; a < ABILITIES; a++)
				if (rune[c][a] != EquipmentState::NotFound) n++;
		return n;
	}

	bool EquipmentSystem::GiveRandomRune(std::mt19937 &rng, int &category, int &ability)
	{
		/* FairyChestObj.OpenChest: any rune not found yet. */
		if (TotalRunesFound() >= CATEGORIES * ABILITIES) return false;
		std::vector<std::pair<int, int>> possible;
		for (int c = 0; c < CATEGORIES; c++)
			for (int a = 0; a < ABILITIES; a++)
				if (rune[c][a] == EquipmentState::NotFound)
					possible.push_back({ c, a });
		if (possible.empty()) return false;
		const auto &chosen = possible[std::uniform_int_distribution<int>(0, (int) possible.size() - 1)(rng)];
		category = chosen.first;
		ability = chosen.second;
		rune[category][ability] = EquipmentState::FoundButNotSeen;
		return true;
	}
}
