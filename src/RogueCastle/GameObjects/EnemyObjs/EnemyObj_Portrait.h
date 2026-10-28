/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Portrait.cs:
 * the haunted painting shakes when the player lingers near it, and
 * flies after them when they walk away (the original's weight tables do
 * exactly that). The advanced, expert and miniboss chases open with a
 * ghost projectile spread. The visual shake offset and the miniboss's
 * random inner picture are not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_PORTRAIT_H
#define ROGUE_CASTLE_ENEMY_OBJ_PORTRAIT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Portrait : public EnemyObj
{
public:
	EnemyObj_Portrait(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_SHAKE,
		ACTION_CHASE
	};

	void StartAction(Action action);
	void UpdateAction();
	void FireSpread();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	bool m_shakeOn = false;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}
};

} // namespace RogueCastle

#endif
