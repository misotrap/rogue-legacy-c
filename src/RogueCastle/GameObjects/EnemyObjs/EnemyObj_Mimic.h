/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Mimic.cs: the
 * chest mimic shakes when the player comes near, then hops at them.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_MIMIC_H
#define ROGUE_CASTLE_ENEMY_OBJ_MIMIC_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Mimic : public EnemyObj
{
public:
	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	EnemyObj_Mimic(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	bool m_isAttacking = false;   /* provoked: hit or touched */
	void Provoke();
	enum Action
	{
		ACTION_NONE,
		ACTION_SHAKE,
		ACTION_JUMP
	};

	void StartAction(Action action);
	void UpdateAction();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}
};

} // namespace RogueCastle

#endif
