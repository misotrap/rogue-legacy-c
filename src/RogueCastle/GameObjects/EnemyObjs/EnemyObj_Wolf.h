/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Wolf.cs: the
 * warg chases the player and pounces at them (a leap when they are close
 * or above it). Wolves also have a one-second start delay so they do not
 * aggro the instant the room loads.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_WOLF_H
#define ROGUE_CASTLE_ENEMY_OBJ_WOLF_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Wolf : public EnemyObj
{
public:
	EnemyObj_Wolf(PlayerObj *target, EnemyDifficulty difficulty);
	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	void ResetState() override;

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
		ACTION_WALK,
		ACTION_STOP,
		ACTION_POUNCE,
		ACTION_HIT
	};

	void StartAction(Action action);
	void UpdateAction();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	bool Chasing = false;   /* C# property (walkTowardsLS sets it) */

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_Wolf.cs lines 17, 18, 21. */
	float PounceDelay = 0.3f;
	float PounceLandDelay = 0.5f;
	float m_startDelay = 1;
	float m_startDelayCounter = 0;
};

} // namespace RogueCastle

#endif
