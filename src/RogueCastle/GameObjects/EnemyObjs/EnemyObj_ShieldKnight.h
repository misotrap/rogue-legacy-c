/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_ShieldKnight.cs:
 * the turtle knight walks in a locked facing (he turns with an
 * animation), and blocks frontal sword hits: reduced damage and the
 * player gets knocked back (ShieldKnockback).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SHIELD_KNIGHT_H
#define ROGUE_CASTLE_ENEMY_OBJ_SHIELD_KNIGHT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_ShieldKnight : public EnemyObj
{
public:
	EnemyObj_ShieldKnight(PlayerObj *target, EnemyDifficulty difficulty);
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
		ACTION_STOP,
		ACTION_WALK,
		ACTION_TURN
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

	/* EnemyObj_ShieldKnight.cs lines 16, 17 (per difficulty). */
	Vector2 ShieldKnockback = Vector2(900, 1050);
	float m_blockDmgReduction = 0.6f;
};

} // namespace RogueCastle

#endif
