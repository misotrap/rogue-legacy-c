/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Horse.cs: the
 * fire horse gallops back and forth, turning at the room bounds and at
 * ledges; advanced drops fire puddles behind it. The Expert's orbiting
 * fire shield (CastFireShield, a rotating circle of fires around the
 * horse) is not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_HORSE_H
#define ROGUE_CASTLE_ENEMY_OBJ_HORSE_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Horse : public EnemyObj
{
public:
	EnemyObj_Horse(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

	/* The run never ends; the turns are handled inside. */
	bool ActionComplete() override { return false; }

private:
	void TurnHorse();
	void DropFireProjectile();
	void InitializeOne(EnemyDifficulty difficulty);

	/* EnemyObj_Horse.cs lines 19-21. */
	float m_collisionCheckTimer = 0.5f;
	float m_fireDropTimer = 0.5f;
	float m_fireDropInterval = 0.075f;
	float m_fireDropLifespan = 0.75f;
	bool m_turning = false;
};

} // namespace RogueCastle

#endif
