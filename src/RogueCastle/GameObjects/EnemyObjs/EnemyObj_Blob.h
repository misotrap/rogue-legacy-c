/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Blob.cs: the
 * slime walks towards the player and hops onto them; the advanced and
 * expert versions leave a damage shield behind when they jump. The
 * miniboss (Alexander, the giant blob that splits) uses the same hop with
 * the boss sprites; the splitting itself is not ported yet.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_BLOB_H
#define ROGUE_CASTLE_ENEMY_OBJ_BLOB_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Blob : public EnemyObj
{
public:
	EnemyObj_Blob(PlayerObj *target, EnemyDifficulty difficulty);

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
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_JUMP
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;   /* the phase that ends the action */

	/* The multi-phase jump only ends on its last phase (see the Knight). */
	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	float JumpDelay = 0.4f;   /* EnemyObj_Blob.cs line 34 */
};

} // namespace RogueCastle

#endif
