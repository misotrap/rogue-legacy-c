/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_FireWizard.cs:
 * the fire wizard hovers around the player and channels three fireballs
 * (the held summon grows over the head, then flies at the player). The
 * teleport LogicSet has weight 0 in the original and is not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_FIRE_WIZARD_H
#define ROGUE_CASTLE_ENEMY_OBJ_FIRE_WIZARD_H

#include "EnemyObj.h"

namespace RogueCastle
{

class ProjectileObj;

class EnemyObj_FireWizard : public EnemyObj
{
public:
	EnemyObj_FireWizard(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_FireWizard() override;
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
		ACTION_MOVE_TOWARDS,
		ACTION_MOVE_AWAY,
		ACTION_STOP,
		ACTION_CAST
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void SummonFireball();
	void CastFireball();
	void ResetFireball();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	int m_castsLeft = 0;
	ProjectileObj *m_fireballSummon = nullptr;   /* the held fireball */

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_FireWizard.cs lines 18, 19, 22, 25 (per difficulty). */
	float SpellDelay = 0.7f;
	float SpellInterval = 0.5f;
	Vector2 m_spellOffset = Vector2(40, -80);
	float MoveDuration = 1.0f;
};

} // namespace RogueCastle

#endif
