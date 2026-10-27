/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Eyeball.cs: the
 * flying eyeball hovers in place and fires projectiles at the player
 * (homing only in the sense of the original: the heading is locked on the
 * player's position at fire time). The expert fans three shots, the
 * miniboss (Khidr) alternates a cardinal spin, a spray and random shots.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_EYEBALL_H
#define ROGUE_CASTLE_ENEMY_OBJ_EYEBALL_H

#include "EnemyObj.h"
#include "../../../DS2DEngine/GameObjs/SpriteObj.h"

namespace RogueCastle
{

class EnemyObj_Eyeball : public EnemyObj
{
public:
	EnemyObj_Eyeball(PlayerObj *target, EnemyDifficulty difficulty);

	/* C# property PupilOffset (tweened by LockEyeball/UnlockEyeball). */
	int PupilOffset() const { return m_pupilOffset; }
	void PupilOffset(int value) { m_pupilOffset = value; }

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
		ACTION_NOTHING,
		ACTION_FIRE,
		ACTION_CARDINAL_SPIN,
		ACTION_SPRAY,
		ACTION_RANDOM
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void UpdateAction();
	/* ProjectileManager.cs line 75: the heading aims at the target plus
	 * the angle offset (mirrored when the source faces left). */
	void FireAtTarget(float angleOffsetDeg);
	void FireAtAngle(float angleDeg);
	void ThrowFan(float angleOffsetDeg);   /* ThrowThreeProjectiles: 0, +45, -45 */
	void ThrowSprayVolley(int startDeg);   /* ThrowSprayProjectiles: 13 shots, 30 deg apart */
	void ThrowRandomShots();                /* ThrowRandomProjectiles: 1 per 45 deg sector */
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	/* Counters of the multi-phase attacks. */
	int m_endPhase = 0;           /* the phase that ends the action */
	int m_shotsLeft = 0;          /* advanced/expert shots or volleys left */
	int m_fanIndex = 0;            /* cardinal spin step */
	int m_sprayVolley = 0;         /* spray volley number */
	int m_randomShots = 0;         /* random attack shots left */
	float m_fireInterval = 0.15f;  /* delay between the shots of a volley */
	int m_flipper = 1;             /* cardinal spin direction */
	/* LockEyeball/UnlockEyeball tween PupilOffset over 0.5 s. */
	int m_pupilOffsetTarget = 4;

	/* The multi-phase attacks only end on their last phase (see the
	 * Knight): the timer alone would cut a phase short. */
	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}

	DS2DEngine::SpriteObj *m_pupil = nullptr;
	int m_pupilOffset = 4;
	float FireballDelay = 0.5f;   /* EnemyObj_Eyeball.cs line 31 */
};

} // namespace RogueCastle

#endif
