// The manor (skill tree): levels, prices, which skills are visible, and
// the bonuses they give. Port of SkillSystem.cs and SkillObj.cs (TotalCost,
// ModifierAmount) plus the buying rule of SkillScreen.cs. No drawing.
#pragma once

#include "SkillData.h"
#include "../Lineage/Lineage.h"

#include <vector>

namespace Skills
{
	class SkillTree
	{
	public:
		SkillTree();

		// New game: every skill at level 0, only the starting skill shown
		// (SkillSystem.ResetAllTraits)
		void Reset();

		static const SkillInfo *Info(SkillType t);
		int Level(SkillType t) const;
		bool Visible(SkillType t) const;
		int PlayerLevel() const { return playerLevel; }

		// SkillObj.TotalCost: base + level * appreciation + 10 * player level
		int TotalCost(SkillType t) const;
		// SkillObj.ModifierAmount: level * per-level bonus (SkillSystem.GetSkill(x).ModifierAmount)
		float ModifierAmount(SkillType t) const;
		bool IsMaxed(SkillType t) const;

		// Buys one level if it is shown, not maxed and affordable: takes the
		// gold, raises the skill and the player level, and shows the
		// neighbouring skills of the manor grid (SkillScreen.SetVisible).
		bool Buy(SkillType t, int &gold);

		// Neighbours on the manor grid: right, left, top, bottom
		// (SkillSystem.GetAllConnectingTraits)
		static std::vector<SkillType> Connecting(SkillType t);
		// Manor building piece revealed by a skill, -1 if none
		static int ManorPiece(SkillType t);

		// Classes the heirs can have (feeds Lineage::Generator)
		Lineage::Unlocks ClassUnlocks(bool traitorUnlocked = false) const;

		// For save files: one level and one visibility flag per skill, in
		// the order of SKILLS
		std::vector<uint8_t> levels;
		std::vector<uint8_t> visible;
		int playerLevel = 0;

	private:
		static int Index(SkillType t);
	};
}
