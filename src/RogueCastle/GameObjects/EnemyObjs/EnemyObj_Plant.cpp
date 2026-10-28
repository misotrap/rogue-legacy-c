/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Plant.cs. */
#include "EnemyObj_Plant.h"
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

EnemyObj_Plant::EnemyObj_Plant(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyPlantIdle_Character", target, difficulty)
{
	Type = EnemyType::Plant;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Plant.cs line 34), so they collapse into one. */
void EnemyObj_Plant::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Plant_Miniboss_Name;
		MaxHealth = EnemyEV::Plant_Miniboss_MaxHealth;
		Damage = EnemyEV::Plant_Miniboss_Damage;
		XPValue = EnemyEV::Plant_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Plant_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Plant_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Plant_Miniboss_DropChance;
		Speed = EnemyEV::Plant_Miniboss_Speed;
		TurnSpeed = EnemyEV::Plant_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Plant_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Plant_Miniboss_Jump;
		CooldownTime = EnemyEV::Plant_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Plant_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Plant_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Plant_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Plant_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Plant_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Plant_Miniboss_Scale.X, EnemyEV::Plant_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Plant_Miniboss_ProjectileScale.X, EnemyEV::Plant_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Plant_Miniboss_Tint.R, EnemyEV::Plant_Miniboss_Tint.G, EnemyEV::Plant_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Plant_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Plant_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Plant_Miniboss_EngageRadius;
		KnockBack = Vector2(EnemyEV::Plant_Miniboss_KnockBack.X, EnemyEV::Plant_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Plant_Expert_Name;
		MaxHealth = EnemyEV::Plant_Expert_MaxHealth;
		Damage = EnemyEV::Plant_Expert_Damage;
		XPValue = EnemyEV::Plant_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Plant_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Plant_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Plant_Expert_DropChance;
		Speed = EnemyEV::Plant_Expert_Speed;
		TurnSpeed = EnemyEV::Plant_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Plant_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Plant_Expert_Jump;
		CooldownTime = EnemyEV::Plant_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Plant_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Plant_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Plant_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Plant_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Plant_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Plant_Expert_Scale.X, EnemyEV::Plant_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Plant_Expert_ProjectileScale.X, EnemyEV::Plant_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Plant_Expert_Tint.R, EnemyEV::Plant_Expert_Tint.G, EnemyEV::Plant_Expert_Tint.B));
		MeleeRadius = EnemyEV::Plant_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Plant_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Plant_Expert_EngageRadius;
		KnockBack = Vector2(EnemyEV::Plant_Expert_KnockBack.X, EnemyEV::Plant_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Plant_Advanced_Name;
		MaxHealth = EnemyEV::Plant_Advanced_MaxHealth;
		Damage = EnemyEV::Plant_Advanced_Damage;
		XPValue = EnemyEV::Plant_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Plant_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Plant_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Plant_Advanced_DropChance;
		Speed = EnemyEV::Plant_Advanced_Speed;
		TurnSpeed = EnemyEV::Plant_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Plant_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Plant_Advanced_Jump;
		CooldownTime = EnemyEV::Plant_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Plant_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Plant_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Plant_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Plant_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Plant_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Plant_Advanced_Scale.X, EnemyEV::Plant_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Plant_Advanced_ProjectileScale.X, EnemyEV::Plant_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Plant_Advanced_Tint.R, EnemyEV::Plant_Advanced_Tint.G, EnemyEV::Plant_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Plant_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Plant_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Plant_Advanced_EngageRadius;
		KnockBack = Vector2(EnemyEV::Plant_Advanced_KnockBack.X, EnemyEV::Plant_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Plant_Basic_Name;
		MaxHealth = EnemyEV::Plant_Basic_MaxHealth;
		Damage = EnemyEV::Plant_Basic_Damage;
		XPValue = EnemyEV::Plant_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Plant_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Plant_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Plant_Basic_DropChance;
		Speed = EnemyEV::Plant_Basic_Speed;
		TurnSpeed = EnemyEV::Plant_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Plant_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Plant_Basic_Jump;
		CooldownTime = EnemyEV::Plant_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Plant_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Plant_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Plant_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Plant_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Plant_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Plant_Basic_Scale.X, EnemyEV::Plant_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Plant_Basic_ProjectileScale.X, EnemyEV::Plant_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Plant_Basic_Tint.R, EnemyEV::Plant_Basic_Tint.G, EnemyEV::Plant_Basic_Tint.B));
		MeleeRadius = EnemyEV::Plant_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Plant_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Plant_Basic_EngageRadius;
		KnockBack = Vector2(EnemyEV::Plant_Basic_KnockBack.X, EnemyEV::Plant_Basic_KnockBack.Y);
		break;
	}
	ProjectileDamage = Damage;
}

void EnemyObj_Plant::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Plant::Spit(float angleDeg)
{
	/* The ProjectileData (line 185): a weighted projectile at a fixed
	 * upward angle (negative degrees = up). */
	float rad = angleDeg * PI / 180.0f;
	Vector2 heading(cosf(rad), sinf(rad));
	m_levelScreen->FireProjectile("PlantProjectile_Sprite", Position(), heading,
	                              ProjectileSpeed, (int) ProjectileDamage,
	                              ProjectileScale, true, this);
}

void EnemyObj_Plant::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_SPIT:
		/* spitProjectileLS (line 163): the attack animation, the spit,
		 * the last frames, back to idle. */
		Audio::PlaySound("Enemy_Venus_Attack_01");   /* EnemyObj_Plant.cs line 176 */
		ChangeSprite("EnemyPlantAttack_Character");
		PlayAnimation(1, TotalFrames() - 1, false);
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Plant::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_SPIT:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			/* The spit fans (lines 166, 178, 200, 219): basic 3 shots,
			 * advanced 5, expert 9, miniboss 7. */
			switch (DifficultyType)
			{
			case MINIBOSS:
				Spit(-60); Spit(-87); Spit(-90); Spit(-93);
				Spit(-75); Spit(-105); Spit(-120);
				break;
			case EXPERT:
				Spit(-45); Spit(-60); Spit(-85); Spit(-90); Spit(-95);
				Spit(-75); Spit(-105); Spit(-120); Spit(-135);
				break;
			case ADVANCED:
				Spit(-60); Spit(-90); Spit(-75); Spit(-105); Spit(-120);
				break;
			default:
				Spit(-90); Spit(-75); Spit(-105);
				break;
			}
			/* PlayAnimation(TotalFrames - 1, TotalFrames). */
			PlayAnimation(TotalFrames() - 1, TotalFrames(), false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ChangeSprite("EnemyPlantIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Plant::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Plant::RunBasicLogic()
{
	/* EnemyObj_Plant.cs line 323: spit whenever the player is within
	 * the radii; nothing in WANDER (the cooldown walkStop is handled by
	 * the base cooldown gating). */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		StartAction(ACTION_SPIT);
		break;
	default:
		break;
	}
}

void EnemyObj_Plant::RunAdvancedLogic()
{
	RunBasicLogic();
}

void EnemyObj_Plant::RunExpertLogic()
{
	RunBasicLogic();
}

void EnemyObj_Plant::RunMinibossLogic()
{
	/* Line 379: the miniboss also spits while the player wanders. */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	case STATE_WANDER:
		StartAction(ACTION_SPIT);
		break;
	default:
		break;
	}
}

} // namespace RogueCastle
