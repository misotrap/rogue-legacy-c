/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Skeleton.cs:
 * patrols, walks to the player, throws bones (far / high), jumps and
 * throws for the harder difficulties. The C# LogicBlocks become a compact
 * action machine with the original probability tables.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SKELETON_H
#define ROGUE_CASTLE_ENEMY_OBJ_SKELETON_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Skeleton : public EnemyObj
{
public:
	EnemyObj_Skeleton(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override;
	void RunExpertLogic() override;
	void RunMinibossLogic() override;
	void UpdateEnemy(GameTime *gameTime) override;

private:
	/* The skeleton's actions (the C# LogicSets). */
	enum Action
	{
		ACTION_NONE,
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_THROW_FAR,
		ACTION_THROW_HIGH,
		ACTION_JUMP_THROW
	};

	/* Picks an action from a weighted list (RunLogicBlock percentages). */
	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void ThrowBones(int count, float angleCentre);   /* FireProjectileLogicAction */
	void UpdateAction();

	Action m_action = ACTION_NONE;
	bool ActionComplete() override
	{
		/* The throws end after their last animation phase, the walks on
		 * their timer (the base rule). */
		if (m_action == ACTION_THROW_FAR || m_action == ACTION_THROW_HIGH)
			return m_actionPhase >= 2 && !IsAnimating();
		if (m_action == ACTION_JUMP_THROW)
			return m_actionPhase >= 2 && !IsAnimating();
		return m_actionTimer <= 0;
	}
	float AttackDelay = 0.1f;
	float JumpDelay = 0.25f;
};

} // namespace RogueCastle

#endif
