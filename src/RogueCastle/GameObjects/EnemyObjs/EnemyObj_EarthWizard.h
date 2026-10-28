/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_EarthWizard.cs:
 * the earth wizard channels under the player's feet and erupts a rock
 * spike there for SpellDuration. The miniboss (Alexander's Sol Mage in
 * the original mixes fire and ice casts too; the port gives it the earth
 * spike and the fireballs, the ice variant is left out).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_EARTH_WIZARD_H
#define ROGUE_CASTLE_ENEMY_OBJ_EARTH_WIZARD_H

#include "EnemyObj.h"

namespace RogueCastle
{

class ProjectileObj;

class EnemyObj_EarthWizard : public EnemyObj
{
public:
	EnemyObj_EarthWizard(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_EarthWizard() override;
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override;
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_MOVE_TOWARDS,
		ACTION_MOVE_AWAY,
		ACTION_STOP,
		ACTION_CAST_EARTH,
		ACTION_CAST_FIRE
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void CancelEarthSpell();
	void CastFireball();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	int m_fireCastsLeft = 0;
	ProjectileObj *m_earthSummon = nullptr;   /* the channel marker */

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_EarthWizard.cs lines 19, 20, 23, 24 (per difficulty). */
	float SpellDelay = 0.3f;
	float SpellDuration = 0.75f;
	float SpellFireDelay = 1.5f;
	float SpellFireInterval = 0.2f;
	Vector2 m_spellOffset = Vector2(40, -80);
	float MoveDuration = 1.0f;
};

} // namespace RogueCastle

#endif
