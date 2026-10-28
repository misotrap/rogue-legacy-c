/* Port of RogueCastle/src/ChestConditionChecker.cs: the fairy chest
 * conditions (kill all enemies, don't look, no jumping...), checked every
 * frame while the chest is locked. */
#ifndef ROGUE_CASTLE_CHEST_CONDITION_CHECKER_H
#define ROGUE_CASTLE_CHEST_CONDITION_CHECKER_H

namespace RogueCastle
{

class FairyChestObj;
class PlayerObj;
class ProceduralLevelScreen;

class ChestConditionChecker
{
public:
	static const int STATE_LOCKED = 0;
	static const int STATE_FREE = 1;
	static const int STATE_FAILED = 2;

	static void SetConditionState(FairyChestObj *chest, PlayerObj *player,
	                              ProceduralLevelScreen *level);
};

}

#endif
