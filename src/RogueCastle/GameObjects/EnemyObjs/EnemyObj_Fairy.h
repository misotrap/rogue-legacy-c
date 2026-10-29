/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Fairy.cs: the
 * fairy ghost flies around the player and fires four-way ghost
 * projectiles. The miniboss (Fairy Godmother) periodically summoning
 * more fairies is not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_FAIRY_H
#define ROGUE_CASTLE_ENEMY_OBJ_FAIRY_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Fairy : public EnemyObj
{
public:
	EnemyObj_Fairy(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_CHASE,
		ACTION_AWAY,
		ACTION_STOP,
		ACTION_FIRE
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

	/* EnemyObj_Fairy.cs line 21. */
	float AttackDelay = 0.5f;
};

} // namespace RogueCastle

#endif
