/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Fireball.cs:
 * the flying flame chases the player and dashes at them. The dash
 * variant that fires four projectiles (advanced/expert
 * dashTowardsFourProjectilesLS) and the Ghost Boss miniboss's fire
 * trail are not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_FIREBALL_H
#define ROGUE_CASTLE_ENEMY_OBJ_FIREBALL_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Fireball : public EnemyObj
{
public:
	EnemyObj_Fireball(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override;
	void RunExpertLogic() override { RunAdvancedLogic(); }
	void RunMinibossLogic() override { RunAdvancedLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_CHASE,
		ACTION_STOP,
		ACTION_DASH
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void StartCooldownMove(int move) override;
	void UpdateAction();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_Fireball.cs lines 22-24, 46-48 (per difficulty). */
	float DashDelay = 0;
	float DashSpeed = 0;
	float DashDuration = 0;
};

} // namespace RogueCastle

#endif
