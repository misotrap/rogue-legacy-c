/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_HomingTurret.cs. */
#include "EnemyObj_HomingTurret.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_HomingTurret::EnemyObj_HomingTurret(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyHomingTurret_Character", target, difficulty)
{
	/* EnemyObj_HomingTurret.cs line 349 (constructor). */
	StopAnimation();
	ForceDraw(true);
	Type = EnemyType::HomingTurret;
}

void EnemyObj_HomingTurret::InitializeEV()
{
	/* EnemyObj_HomingTurret.cs line 22. */
	switch (DifficultyType)
	{
	case MINIBOSS:
		Name = EnemyEV::HomingTurret_Miniboss_Name;
		MaxHealth = EnemyEV::HomingTurret_Miniboss_MaxHealth;
		Damage = EnemyEV::HomingTurret_Miniboss_Damage;
		XPValue = EnemyEV::HomingTurret_Miniboss_XPValue;
		AlwaysFaceTarget = EnemyEV::HomingTurret_Miniboss_AlwaysFaceTarget;
		IsWeightedFlag = EnemyEV::HomingTurret_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::HomingTurret_Miniboss_Scale.X, EnemyEV::HomingTurret_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::HomingTurret_Miniboss_ProjectileScale.X, EnemyEV::HomingTurret_Miniboss_ProjectileScale.Y);
		ProjectileSpeed = EnemyEV::HomingTurret_Miniboss_ProjectileSpeed;
		ProjectileDamage = Damage;
		m_fireDelay = 2.25f;
		break;
	case EXPERT:
		Name = EnemyEV::HomingTurret_Expert_Name;
		MaxHealth = EnemyEV::HomingTurret_Expert_MaxHealth;
		Damage = EnemyEV::HomingTurret_Expert_Damage;
		XPValue = EnemyEV::HomingTurret_Expert_XPValue;
		AlwaysFaceTarget = EnemyEV::HomingTurret_Expert_AlwaysFaceTarget;
		IsWeightedFlag = EnemyEV::HomingTurret_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::HomingTurret_Expert_Scale.X, EnemyEV::HomingTurret_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::HomingTurret_Expert_ProjectileScale.X, EnemyEV::HomingTurret_Expert_ProjectileScale.Y);
		ProjectileSpeed = EnemyEV::HomingTurret_Expert_ProjectileSpeed;
		ProjectileDamage = Damage;
		m_fireDelay = 1.5f;
		break;
	case ADVANCED:
		Name = EnemyEV::HomingTurret_Advanced_Name;
		MaxHealth = EnemyEV::HomingTurret_Advanced_MaxHealth;
		Damage = EnemyEV::HomingTurret_Advanced_Damage;
		XPValue = EnemyEV::HomingTurret_Advanced_XPValue;
		AlwaysFaceTarget = EnemyEV::HomingTurret_Advanced_AlwaysFaceTarget;
		IsWeightedFlag = EnemyEV::HomingTurret_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::HomingTurret_Advanced_Scale.X, EnemyEV::HomingTurret_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::HomingTurret_Advanced_ProjectileScale.X, EnemyEV::HomingTurret_Advanced_ProjectileScale.Y);
		ProjectileSpeed = EnemyEV::HomingTurret_Advanced_ProjectileSpeed;
		ProjectileDamage = Damage;
		break;
	default: /* BASIC */
		Name = EnemyEV::HomingTurret_Basic_Name;
		MaxHealth = EnemyEV::HomingTurret_Basic_MaxHealth;
		Damage = EnemyEV::HomingTurret_Basic_Damage;
		XPValue = EnemyEV::HomingTurret_Basic_XPValue;
		AlwaysFaceTarget = EnemyEV::HomingTurret_Basic_AlwaysFaceTarget;
		IsWeightedFlag = EnemyEV::HomingTurret_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::HomingTurret_Basic_Scale.X, EnemyEV::HomingTurret_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::HomingTurret_Basic_ProjectileScale.X, EnemyEV::HomingTurret_Basic_ProjectileScale.Y);
		ProjectileSpeed = EnemyEV::HomingTurret_Basic_ProjectileSpeed;
		ProjectileDamage = Damage;
		m_fireDelay = 2.0f;
		break;
	}

	/* InitializeLogic (line 139): the rate comes from the room Tag, the
	 * rotation is the firing angle. */
	float delay = ParseTagToFloat("delay");
	if (delay > 0)
		m_fireDelay = delay;
}

void EnemyObj_HomingTurret::RunBasicLogic()
{
	/* Line 283: fire whatever the state; the action never ends, so this
	 * only starts it once. */
	if (!m_actionActive)
	{
		m_actionActive = true;
		m_actionTimer = m_fireDelay;
	}
}

void EnemyObj_HomingTurret::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	if (m_actionActive && m_actionTimer <= 0)
	{
		/* FireProjectileLogicAction: along the rotation, from the
		 * SourceAnchor (35, 0). */
		float rad = Rotation() * PI / 180.0f;
		Vector2 heading(cosf(rad), sinf(rad));
		Audio::PlaySound("Turret_Attack01", "Turret_Attack02", "Turret_Attack03");   /* the C# fireProjectileLS */
		m_levelScreen->FireProjectile("HomingProjectile_Sprite",
		                              Position() + Vector2(35, 0), heading,
		                              ProjectileSpeed, (int) Damage,
		                              ProjectileScale, false, this);
		m_actionTimer = m_fireDelay;
	}
}

} // namespace RogueCastle
