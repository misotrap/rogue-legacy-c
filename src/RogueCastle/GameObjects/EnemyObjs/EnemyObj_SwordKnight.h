/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SwordKnight.cs:
 * the sword knight closes in and slashes; the advanced and expert
 * versions send a sword wave (EnemySpearKnightWave_Sprite) and the
 * expert's triple attack dashes with three slashes first.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SWORD_KNIGHT_H
#define ROGUE_CASTLE_ENEMY_OBJ_SWORD_KNIGHT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_SwordKnight : public EnemyObj
{
public:
	EnemyObj_SwordKnight(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_ATTACK,
		ACTION_ATTACK_TRIPLE
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void StartCooldownMove(int move) override;
	void UpdateAction();
	void FireWave();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	/* Whether the slash fires the sword wave (Advanced/Expert attackLS
	 * and the Expert triple). */
	bool m_attackFiresWave = false;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_SwordKnight.cs lines 17-19, set per difficulty. */
	float SlashDelay = 0.25f;
	float SlashTripleDelay = 1.25f;
	float TripleAttackSpeed = 500;
};

} // namespace RogueCastle

#endif
