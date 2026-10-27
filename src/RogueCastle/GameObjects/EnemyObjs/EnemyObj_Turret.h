/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Turret.cs: the
 * wall turret fires a projectile along its rotation every "delay" seconds
 * (both read from the room object: Rotation and Tag "delay:...,speed:...").
 * It cannot be killed and ignores the player's position.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_TURRET_H
#define ROGUE_CASTLE_ENEMY_OBJ_TURRET_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Turret : public EnemyObj
{
public:
	EnemyObj_Turret(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

	/* fireProjectileLS loops forever (RunLogicBlock(false, ...)), so the
	 * action never completes; UpdateEnemy re-arms the timer. */
	bool ActionComplete() override { return false; }

private:
	float m_fireDelay = 2.0f;
	float m_fireSpeed = 0;
};

} // namespace RogueCastle

#endif
