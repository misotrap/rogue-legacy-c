/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Starburst.cs:
 * the star turret fires a cross of turret projectiles when the player
 * is in range (the advanced's is rotated 45 degrees, the expert fires
 * eight). The star's spin animation between volleys is not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_STARBURST_H
#define ROGUE_CASTLE_ENEMY_OBJ_STARBURST_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Starburst : public EnemyObj
{
public:
	EnemyObj_Starburst(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_FIRE,
		ACTION_NOTHING
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

	/* EnemyObj_Starburst.cs line 16. */
	float FireballDelay = 0.5f;
};

} // namespace RogueCastle

#endif
