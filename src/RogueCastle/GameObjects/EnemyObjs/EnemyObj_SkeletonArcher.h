/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SkeletonArcher.cs:
 * the archer skeleton fires arcing arrows (weighted, so they drop):
 * straight, medium arch and high arch shots, more arrows per difficulty.
 * The original also rotates its bow part to the launch angle
 * (AngleArcher); the port skips that visual (no part rotation yet).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SKELETON_ARCHER_H
#define ROGUE_CASTLE_ENEMY_OBJ_SKELETON_ARCHER_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_SkeletonArcher : public EnemyObj
{
public:
	EnemyObj_SkeletonArcher(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_STOP,
		ACTION_FIRE_STRAIGHT,
		ACTION_FIRE_HIGH_ARCH,
		ACTION_FIRE_ARCH
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void FireArrow(float angleDeg);
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	int m_shotsLeft = 0;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_SkeletonArcher.cs line 14. */
	float m_fireDelay = 0.5f;
};

} // namespace RogueCastle

#endif
