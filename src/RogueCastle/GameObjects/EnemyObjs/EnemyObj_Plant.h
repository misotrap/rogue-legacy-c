/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Plant.cs: the
 * venus flytrap spits weighted (arcing) projectiles upward at fixed
 * angles; the higher the difficulty the wider the fan. The projectile
 * dies on terrain in the original (CollidesWithTerrain); the port's
 * projectiles do not collide with terrain yet.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_PLANT_H
#define ROGUE_CASTLE_ENEMY_OBJ_PLANT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Plant : public EnemyObj
{
public:
	EnemyObj_Plant(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_SPIT
	};

	void StartAction(Action action);
	void UpdateAction();
	void Spit(float angleDeg);
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
