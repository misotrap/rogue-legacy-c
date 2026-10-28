/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Ninja.cs: the
 * ninja runs around, throws shuriken at the player (a fan at higher
 * difficulties) and teleports to the ceiling when hit. The teleport's
 * log decoy and smoke sprites are visual only; the port keeps the
 * disappear/reappear above the player.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_NINJA_H
#define ROGUE_CASTLE_ENEMY_OBJ_NINJA_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Ninja : public EnemyObj
{
public:
	EnemyObj_Ninja(PlayerObj *target, EnemyDifficulty difficulty);
	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override;
	void RunExpertLogic() override;
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_THROW,
		ACTION_TELEPORT
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	void ThrowShuriken(float angleOffsetDeg);
	class TerrainObj *FindClosestCeiling();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	TerrainObj *m_closestCeiling = nullptr;

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	/* EnemyObj_Ninja.cs lines 24-28, 25 (ChanceToTeleport per
	 * difficulty). */
	float TeleportDelay = 0.35f;
	float PauseBeforeProjectile = 0.45f;
	float PauseAfterProjectile = 0.45f;
	float ChanceToTeleport = 0.35f;
};

} // namespace RogueCastle

#endif
