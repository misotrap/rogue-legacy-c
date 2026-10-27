// The blacksmith (blueprints, armour) and the enchantress (runes): what the
// player has found, bought and equipped, prices, weight, and the bonuses it
// all gives. Port of EquipmentSystem.cs, the buying/equipping rules of
// BlacksmithScreen.cs / EnchantressScreen.cs, PlayerObj.MaxWeight and the
// blueprint choice of ChestObj.GivePrint. No drawing.
#pragma once

#include "EquipmentData.h"

#include <cstdint>
#include <random>

namespace Equipment
{
	constexpr int CATEGORIES = EquipmentCategoryType::Total;  // sword, helm, chest, limbs, cape
	constexpr int BASE_TYPES = EquipmentBaseType::Total;      // bronze .. dark (15)
	constexpr int ABILITIES = EquipmentAbilityType::Total;    // runes (11)

	// Sum of everything equipped (PlayerObj stats read these)
	struct Totals
	{
		int weight = 0;
		int health = 0, damage = 0, armor = 0, mana = 0, magic = 0;
		float secondary[EquipmentSecondaryDataType::Total] = {}; // per EquipmentSecondaryDataType
		int runes[ABILITIES] = {};                                // equipped runes of each kind
	};

	class EquipmentSystem
	{
	public:
		EquipmentSystem();

		static const ItemData *Item(int category, int index);

		// Game.PlayerStats.GetBlueprintArray / GetRuneArray (EquipmentState::)
		uint8_t blueprint[CATEGORIES][BASE_TYPES];
		uint8_t rune[CATEGORIES][ABILITIES];
		int8_t equipped[CATEGORIES];      // base type index, -1 = nothing
		int8_t equippedRune[CATEGORIES];  // ability index, -1 = nothing
		int totalRunesPurchased = 0;      // PlayerStats.TotalRunesPurchased

		void Reset();

		// Blacksmith: buy a found blueprint (gold -= Cost)
		bool BuyBlueprint(int category, int index, int &gold);
		// Equip a bought item if the total weight stays within maxWeight;
		// equipping the item already worn takes it off
		bool Equip(int category, int index, int maxWeight);

		// Enchantress: rune cost grows with every rune bought
		int RuneCost(int category, int ability) const;
		bool BuyRune(int category, int ability, int &gold);
		bool EquipRune(int category, int ability);

		Totals ComputeTotals() const;
		int CurrentWeight() const { return ComputeTotals().weight; }

		// PlayerObj.MaxWeight: base weight + Equip_Up skill bonus + weight
		// stat drops * GameEV.ITEM_STAT_WEIGHT_AMOUNT (the game passes them in)
		static int MaxWeight(int baseWeight, float equipUpModifier, float equipUpFinalModifier, int bonusWeightDrops, int itemStatWeightAmount);

		// ChestObj.GivePrint: a random blueprint not found yet that this
		// chest level and colour can give; marks it FoundButNotSeen.
		// Returns false when none is left (the chest gives gold instead).
		bool GiveRandomBlueprint(int chestLevel, int chestType, std::mt19937 &rng, int &category, int &index);
		int TotalBlueprintsFound() const;
	};
}
