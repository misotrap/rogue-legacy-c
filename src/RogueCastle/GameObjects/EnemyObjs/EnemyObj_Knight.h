/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Knight.cs: the
 * corrupt knight patrols, closes in, then either charges with a spear
 * thrust or throws a spear wave. Probability tables and timings from the
 * original LogicBlocks.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_KNIGHT_H
#define ROGUE_CASTLE_ENEMY_OBJ_KNIGHT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Knight : public EnemyObj
{
public:
	EnemyObj_Knight(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override;
	void RunExpertLogic() override;
	void RunMinibossLogic() override;
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_THRUST,
		ACTION_THROW_SPEAR
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void StartCooldownMove(int move) override;
	void UpdateAction();
	void FireSpear(int count, float angleCentre);
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	bool ActionComplete() override
	{
		if (m_action == ACTION_THRUST)
			return m_actionPhase >= 3 && m_actionTimer <= 0;
		if (m_action == ACTION_THROW_SPEAR)
			return m_actionPhase >= 3 && m_actionTimer <= 0;
		return m_actionTimer <= 0;
	}
	/* EnemyObj_Knight.cs line 19: the thrust timings per difficulty. */
	float AttackThrustDelay = 0.65f;
	float AttackThrustSpeed = 1850;
	float AttackThrustDuration = 0.4f;
	float AttackProjectileDelay = 0.35f;
};

} // namespace RogueCastle

#endif
