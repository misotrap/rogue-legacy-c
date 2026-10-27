#include "SkillTree.h"

#include "../Types/ClassType.h"

namespace Skills
{
	SkillTree::SkillTree()
	{
		Reset();
	}

	int SkillTree::Index(SkillType t)
	{
		for (int i = 0; i < SKILL_COUNT; i++)
			if (SKILLS[i].type == t) return i;
		return -1;
	}

	const SkillInfo *SkillTree::Info(SkillType t)
	{
		int i = Index(t);
		return i >= 0 ? &SKILLS[i] : nullptr;
	}

	void SkillTree::Reset()
	{
		levels.assign(SKILL_COUNT, 0);
		visible.assign(SKILL_COUNT, 0);
		visible[Index(STARTING_SKILL)] = 1;
		playerLevel = 0;
	}

	int SkillTree::Level(SkillType t) const
	{
		int i = Index(t);
		return i >= 0 ? levels[i] : 0;
	}

	bool SkillTree::Visible(SkillType t) const
	{
		int i = Index(t);
		return i >= 0 && visible[i] != 0;
	}

	int SkillTree::TotalCost(SkillType t) const
	{
		const SkillInfo *s = Info(t);
		if (s == nullptr) return 0;
		return s->baseCost + Level(t) * s->appreciation + SKILL_LEVEL_COST_INCREASE * playerLevel;
	}

	float SkillTree::ModifierAmount(SkillType t) const
	{
		const SkillInfo *s = Info(t);
		return s != nullptr ? Level(t) * s->perLevelModifier : 0.0f;
	}

	bool SkillTree::IsMaxed(SkillType t) const
	{
		const SkillInfo *s = Info(t);
		return s == nullptr || Level(t) >= s->maxLevel;
	}

	bool SkillTree::Buy(SkillType t, int &gold)
	{
		int i = Index(t);
		if (i < 0 || !visible[i] || IsMaxed(t)) return false;
		int cost = TotalCost(t);
		if (gold < cost) return false;
		gold -= cost;
		// SkillSystem.LevelUpTrait
		playerLevel++;
		levels[i]++;
		for (SkillType n : Connecting(t))
		{
			int j = Index(n);
			if (j >= 0) visible[j] = 1;
		}
		return true;
	}

	std::vector<SkillType> SkillTree::Connecting(SkillType t)
	{
		std::vector<SkillType> out;
		for (int r = 0; r < GRID_ROWS; r++)
			for (int c = 0; c < GRID_COLS; c++)
			{
				if (SKILL_GRID[r][c] != t) continue;
				const int dr[] = { 0, 0, -1, 1 }, dc[] = { 1, -1, 0, 0 }; // right, left, top, bottom
				for (int k = 0; k < 4; k++)
				{
					int rr = r + dr[k], cc = c + dc[k];
					if (rr < 0 || rr >= GRID_ROWS || cc < 0 || cc >= GRID_COLS) continue;
					SkillType n = SKILL_GRID[rr][cc];
					if (n != SkillType::Null) out.push_back(n);
				}
				return out;
			}
		return out;
	}

	int SkillTree::ManorPiece(SkillType t)
	{
		for (int r = 0; r < GRID_ROWS; r++)
			for (int c = 0; c < GRID_COLS; c++)
				if (SKILL_GRID[r][c] == t) return MANOR_PIECE[r][c];
		return -1;
	}

	Lineage::Unlocks SkillTree::ClassUnlocks(bool traitorUnlocked) const
	{
		// ClassType.GetRandomClass / ClassType.Upgraded
		Lineage::Unlocks u;
		u.ninja = Level(SkillType::Ninja_Unlock) > 0;
		u.banker = Level(SkillType::Banker_Unlock) > 0;
		u.spellSword = Level(SkillType::Spellsword_Unlock) > 0;
		u.lich = Level(SkillType::Lich_Unlock) > 0;
		u.dragon = Level(SkillType::SuperSecret) > 0;
		u.traitor = traitorUnlocked;
		u.upgraded[ClassType::Knight] = Level(SkillType::Knight_Up) > 0;
		u.upgraded[ClassType::Wizard] = Level(SkillType::Mage_Up) > 0;
		u.upgraded[ClassType::Barbarian] = Level(SkillType::Barbarian_Up) > 0;
		u.upgraded[ClassType::Ninja] = Level(SkillType::Ninja_Up) > 0;
		u.upgraded[ClassType::Assassin] = Level(SkillType::Assassin_Up) > 0;
		u.upgraded[ClassType::Banker] = Level(SkillType::Banker_Up) > 0;
		u.upgraded[ClassType::SpellSword] = Level(SkillType::SpellSword_Up) > 0;
		u.upgraded[ClassType::Lich] = Level(SkillType::Lich_Up) > 0;
		return u;
	}
}
