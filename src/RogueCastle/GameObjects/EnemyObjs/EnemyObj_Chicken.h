/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Chicken.cs:
 * the golden chicken flutters left and right around the room (one hit
 * kills it - it drops the gold).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_CHICKEN_H
#define ROGUE_CASTLE_ENEMY_OBJ_CHICKEN_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Chicken : public EnemyObj
{
public:
	EnemyObj_Chicken(PlayerObj *target, EnemyDifficulty difficulty);

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_RUN_LEFT,
		ACTION_RUN_RIGHT
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
};

} // namespace RogueCastle

#endif
