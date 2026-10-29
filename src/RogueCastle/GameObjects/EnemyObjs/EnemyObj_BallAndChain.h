/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_BallAndChain.cs:
 * the flail knight walks around with a spiked ball swinging around him
 * (a ProjectileObj orbiting (X, Bounds.Top); advanced and expert get a
 * second, faster ball). The chain links are visual only.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_BALL_AND_CHAIN_H
#define ROGUE_CASTLE_ENEMY_OBJ_BALL_AND_CHAIN_H

#include "EnemyObj.h"

namespace RogueCastle
{

class ProjectileObj;

class EnemyObj_BallAndChain : public EnemyObj
{
public:
	EnemyObj_BallAndChain(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_BallAndChain() override;
	void ResetState() override;
	void Kill(bool giveXP = true) override;

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
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	ProjectileObj *m_ball = nullptr;       /* the swinging flail ball */
	ProjectileObj *m_ball2 = nullptr;      /* advanced/expert second ball */
	float m_ballAngle = 0;
	float m_actualChainRadius = 0;

	/* EnemyObj_BallAndChain.cs lines 20, 25, 32-34 (per difficulty). */
	float ChainSpeed = 2.5f;
	float ChainRadius = 260;
	float ChainSpeed2Modifier = 1.5f;
};

} // namespace RogueCastle

#endif
