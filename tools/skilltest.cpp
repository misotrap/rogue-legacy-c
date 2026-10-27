// Simulates manor purchases over several lives (no graphics):
//   make skilltest && ./skilltest [lives] [gold per life]
#include "../src/RogueCastle/Skills/SkillTree.h"

#include <cstdio>
#include <cstdlib>

using namespace Skills;

int main(int argc, char **argv)
{
	int lives = argc > 1 ? atoi(argv[1]) : 8;
	int goldPerLife = argc > 2 ? atoi(argv[2]) : 1500;
	SkillTree tree;
	int gold = 0;
	for (int life = 1; life <= lives; life++)
	{
		gold += goldPerLife;
		printf("life %d: %d gold\n", life, gold);
		// Buy the cheapest visible skill until nothing is affordable
		for (;;)
		{
			const SkillInfo *best = nullptr;
			for (const SkillInfo &s : SKILLS)
				if (tree.Visible(s.type) && !tree.IsMaxed(s.type) && (best == nullptr || tree.TotalCost(s.type) < tree.TotalCost(best->type)))
					best = &s;
			if (best == nullptr || tree.TotalCost(best->type) > gold) break;
			int cost = tree.TotalCost(best->type);
			tree.Buy(best->type, gold);
			printf("  bought %-20s level %2d for %5d (manor piece %2d)\n", best->name, tree.Level(best->type), cost,
				SkillTree::ManorPiece(best->type));
		}
	}
	Lineage::Unlocks u = tree.ClassUnlocks();
	printf("player level %d, %d gold left; unlocked: ninja %d banker %d spellsword %d lich %d dragon %d, knight+ %d\n",
		tree.PlayerLevel(), gold, u.ninja, u.banker, u.spellSword, u.lich, u.dragon, u.upgraded[0]);
	printf("health bonus %.0f hp, attack bonus %.0f str, mana bonus %.0f mp\n", tree.ModifierAmount(SkillType::Health_Up),
		tree.ModifierAmount(SkillType::Attack_Up), tree.ModifierAmount(SkillType::Mana_Up));
	return 0;
}
