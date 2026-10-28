/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Zombie.cs: the
 * zombie sleeps in the ground, rises when the player comes close, walks
 * towards them, and sinks back down when they leave. The C# re-checks
 * the state every walk cycle (the looping LogicSet pauses between
 * iterations); the port ends the walk action every 0.5 s to get the same
 * re-evaluation.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_ZOMBIE_H
#define ROGUE_CASTLE_ENEMY_OBJ_ZOMBIE_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Zombie : public EnemyObj
{
public:
	EnemyObj_Zombie(PlayerObj *target, EnemyDifficulty difficulty);
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum ZombieState
	{
		ZOMBIE_LOWERED,
		ZOMBIE_RISING,
		ZOMBIE_WALKING,
		ZOMBIE_LOWERING
	};

	void InitializeOne(EnemyDifficulty difficulty);

	/* The rise/walk/lower cycle is driven by the state machine below;
	 * the base's timer-based completion would end (and so restart) the
	 * rise every frame. */
	bool ActionComplete() override { return false; }

	ZombieState m_zombieState = ZOMBIE_LOWERED;
	/* C# properties Risen/Lowered. */
	bool Risen = false;
	bool Lowered = true;
};

} // namespace RogueCastle

#endif
