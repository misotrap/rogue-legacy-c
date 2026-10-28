/* Port of RogueCastle/src/Types/ChestConditionType.cs: what the player
 * must do (or avoid doing) to unlock a fairy chest. */
#ifndef ROGUE_CASTLE_CHEST_CONDITION_TYPE_H
#define ROGUE_CASTLE_CHEST_CONDITION_TYPE_H

namespace RogueCastle
{
namespace ChestConditionType
{
	/* The value comes from the room object's Tag attribute. */
	static const int None = 0;
	static const int KillAllEnemies = 1;
	static const int HealthBelow15 = 2;
	static const int DontLook = 3;
	static const int NoJumping = 4;
	static const int NoSound = 5;
	static const int NoFloor = 6;
	static const int NoAttackingEnemies = 7;
	static const int ReachIn5Seconds = 8;
	static const int TakeNoDamage = 9;
	static const int InvisibleChest = 10;
}
}

#endif
