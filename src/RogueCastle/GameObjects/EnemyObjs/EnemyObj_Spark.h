/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Spark.cs: the
 * spark crawls along the walls and ledges of the room. It walks in its
 * heading and steers with corner/edge probes against the room bounds and
 * the terrain (the original's rotated-slope +-45 degree cases are
 * skipped with the rotated hulls).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SPARK_H
#define ROGUE_CASTLE_ENEMY_OBJ_SPARK_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Spark : public EnemyObj
{
public:
	EnemyObj_Spark(PlayerObj *target, EnemyDifficulty difficulty);
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override {}
	void RunAdvancedLogic() override {}
	void RunExpertLogic() override {}
	void RunMinibossLogic() override {}
	void UpdateEnemy(GameTime *gameTime) override;

private:
	void HookToGround();
	void CollisionCheckRight();
	void InitializeOne(EnemyDifficulty difficulty);

	bool m_hookedToGround = false;
	/* EnemyObj_Spark.cs line 13 (m_collisionBoxSize) and the Orientation
	 * the steering keeps turning to. */
	static const int m_collisionBoxSize = 10;
	float m_orientation = 0;
};

} // namespace RogueCastle

#endif
