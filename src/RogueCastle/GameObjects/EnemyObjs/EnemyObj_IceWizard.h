/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_IceWizard.cs:
 * the ice wizard channels a growing iceball and shatters it into shards
 * radiating all around (random angles for the Expert, in eight waves).
 * Like the fire wizard, its teleport set has weight 0 and is not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_ICE_WIZARD_H
#define ROGUE_CASTLE_ENEMY_OBJ_ICE_WIZARD_H

#include "EnemyObj.h"

namespace RogueCastle
{

class ProjectileObj;

class EnemyObj_IceWizard : public EnemyObj
{
public:
	EnemyObj_IceWizard(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_IceWizard() override;

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
	void SummonIceball();
	void ShatterIceball();
	void ResetIceball();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	int m_shatterWavesLeft = 0;
	ProjectileObj *m_iceballSummon = nullptr;   /* the held iceball */

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_IceWizard.cs lines 17, 20, 22 (per difficulty). */
	float SpellDelay = 0.8f;
	int SpellProjectileCount = 7;
	Vector2 m_spellOffset = Vector2(40, -80);
	float MoveDuration = 1.0f;
};

} // namespace RogueCastle

#endif
