/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_LastBoss.cs
 * (subset): the final boss. The first form is Johannes, a grey mirror
 * of the player (the original uses the player sprites tinted grey):
 * he walks, attacks with the player's sword animation, dashes and
 * throws daggers. When that form falls, the second form takes over:
 * the Fountain (EnemyLastBoss* sprites) walks and casts sword and
 * spear projectiles. The original's cinematics between the forms
 * (screen borders, the player being moved), the orb summons, the
 * damage shields and the Neo variant are not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_LAST_BOSS_H
#define ROGUE_CASTLE_ENEMY_OBJ_LAST_BOSS_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_LastBoss : public EnemyObj
{
public:
	EnemyObj_LastBoss(PlayerObj *target, EnemyDifficulty difficulty);
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
		/* First form (Johannes, the player mirror). */
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_JUMP,
		ACTION_MOVE_ATTACK,
		ACTION_THROW_DAGGER,
		ACTION_DASH,
		/* Second form (the Fountain). */
		ACTION_SF_WALK,
		ACTION_SF_ATTACK,
		ACTION_SF_CAST_SWORDS,
		ACTION_SF_CAST_SPEARS
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void SecondFormTransition();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	bool m_inSecondForm = false;
	bool m_formKilled = false;   /* the form that just died */

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_LastBoss.cs lines 19, 35. */
	float DaggerProjectileSpeed = 900;
	float m_lastBossAttackDelay = 0.35f;
};

} // namespace RogueCastle

#endif
