/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_HomingTurret.cs:
 * like the wall turret (fires along its rotation every Tag 'delay'
 * seconds) but with the HomingProjectile sprite - which, despite the
 * name, does not home (ChaseTarget = false in the original).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_HOMING_TURRET_H
#define ROGUE_CASTLE_ENEMY_OBJ_HOMING_TURRET_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_HomingTurret : public EnemyObj
{
public:
	EnemyObj_HomingTurret(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

	bool ActionComplete() override { return false; }

private:
	float m_fireDelay = 5.0f;   /* C# FireDelay, per difficulty */
};

} // namespace RogueCastle

#endif
