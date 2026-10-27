/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Turret.cs. */
#include "EnemyObj_Turret.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_Turret::EnemyObj_Turret(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyTurretFire_Character", target, difficulty)
{
	/* EnemyObj_Turret.cs line 152 (constructor). */
	IsCollidableFlag = false;
	ForceDraw(true);
	Type = EnemyType::Turret;
	StopAnimation();
	NonKillable = true;
}

void EnemyObj_Turret::InitializeEV()
{
	/* EnemyObj_Turret.cs line 17: the same stats for every difficulty. */
	Scale(Vector2(2.0f, 2.0f));
	AnimationDelay(1 / 10.0f);
	Speed = 0;
	MaxHealth = 10;
	EngageRadius = 30;
	ProjectileRadius = 20;
	MeleeRadius = 10;
	CooldownTime = 2.0f;
	KnockBack = Vector2(1, 2);
	Damage = 25;
	JumpHeight = 20.5f;
	AlwaysFaceTarget = false;
	CanFallOffLedges = false;
	XPValue = 2;
	CanBeKnockedBack = false;
	IsWeightedFlag = false;
	Name = "Wall Turret";

	/* InitializeLogic (line 44): the angle and rate come from the room
	 * object (Rotation and the Tag). */
	m_fireDelay = ParseTagToFloat("delay");
	if (m_fireDelay <= 0)
		m_fireDelay = 10000;   /* the C# error path (line 51) */
	m_fireSpeed = ParseTagToFloat("speed");
}

void EnemyObj_Turret::RunBasicLogic()
{
	/* Line 80: fire whatever the state is; the action never ends, so this
	 * only starts it once. */
	if (!m_actionActive)
	{
		m_actionActive = true;
		m_actionTimer = m_fireDelay;
	}
}

void EnemyObj_Turret::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	if (m_actionActive && m_actionTimer <= 0)
	{
		/* FireProjectileLogicAction: a projectile along the rotation. */
		float rad = Rotation() * PI / 180.0f;
		Vector2 heading(cosf(rad), sinf(rad));
		m_levelScreen->FireProjectile("TurretProjectile_Sprite", Position(),
		                              heading, m_fireSpeed, (int) Damage,
		                              ProjectileScale, false, this);
		m_actionTimer = m_fireDelay;   /* DelayLogicAction(delay) */
	}
}

} // namespace RogueCastle
