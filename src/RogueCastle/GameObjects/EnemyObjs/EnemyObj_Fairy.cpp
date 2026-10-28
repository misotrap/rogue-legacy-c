/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Fairy.cs. */
#include "EnemyObj_Fairy.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Fairy::EnemyObj_Fairy(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyFairyGhostIdle_Character", target, difficulty)
{
	Type = EnemyType::Fairy;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Fairy.cs line 22), so they collapse into one. */
void EnemyObj_Fairy::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Fairy_Miniboss_Name;
		MaxHealth = EnemyEV::Fairy_Miniboss_MaxHealth;
		Damage = EnemyEV::Fairy_Miniboss_Damage;
		XPValue = EnemyEV::Fairy_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Fairy_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fairy_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fairy_Miniboss_DropChance;
		Speed = EnemyEV::Fairy_Miniboss_Speed;
		TurnSpeed = EnemyEV::Fairy_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fairy_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Fairy_Miniboss_Jump;
		CooldownTime = EnemyEV::Fairy_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Fairy_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fairy_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fairy_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fairy_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fairy_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Fairy_Miniboss_Scale.X, EnemyEV::Fairy_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fairy_Miniboss_ProjectileScale.X, EnemyEV::Fairy_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fairy_Miniboss_Tint.R, EnemyEV::Fairy_Miniboss_Tint.G, EnemyEV::Fairy_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Fairy_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Fairy_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Fairy_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fairy_Miniboss_KnockBack.X, EnemyEV::Fairy_Miniboss_KnockBack.Y);
		m_resetSpriteName = "EnemyFairyGhostBossIdle_Character";   /* line 170 */
		break;
	case EXPERT:
		Name = EnemyEV::Fairy_Expert_Name;
		MaxHealth = EnemyEV::Fairy_Expert_MaxHealth;
		Damage = EnemyEV::Fairy_Expert_Damage;
		XPValue = EnemyEV::Fairy_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Fairy_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fairy_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fairy_Expert_DropChance;
		Speed = EnemyEV::Fairy_Expert_Speed;
		TurnSpeed = EnemyEV::Fairy_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fairy_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Fairy_Expert_Jump;
		CooldownTime = EnemyEV::Fairy_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Fairy_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fairy_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fairy_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fairy_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fairy_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Fairy_Expert_Scale.X, EnemyEV::Fairy_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fairy_Expert_ProjectileScale.X, EnemyEV::Fairy_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fairy_Expert_Tint.R, EnemyEV::Fairy_Expert_Tint.G, EnemyEV::Fairy_Expert_Tint.B));
		MeleeRadius = EnemyEV::Fairy_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Fairy_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Fairy_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fairy_Expert_KnockBack.X, EnemyEV::Fairy_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Fairy_Advanced_Name;
		MaxHealth = EnemyEV::Fairy_Advanced_MaxHealth;
		Damage = EnemyEV::Fairy_Advanced_Damage;
		XPValue = EnemyEV::Fairy_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Fairy_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fairy_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fairy_Advanced_DropChance;
		Speed = EnemyEV::Fairy_Advanced_Speed;
		TurnSpeed = EnemyEV::Fairy_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fairy_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Fairy_Advanced_Jump;
		CooldownTime = EnemyEV::Fairy_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Fairy_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fairy_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fairy_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fairy_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fairy_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Fairy_Advanced_Scale.X, EnemyEV::Fairy_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fairy_Advanced_ProjectileScale.X, EnemyEV::Fairy_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fairy_Advanced_Tint.R, EnemyEV::Fairy_Advanced_Tint.G, EnemyEV::Fairy_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Fairy_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Fairy_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Fairy_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fairy_Advanced_KnockBack.X, EnemyEV::Fairy_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Fairy_Basic_Name;
		MaxHealth = EnemyEV::Fairy_Basic_MaxHealth;
		Damage = EnemyEV::Fairy_Basic_Damage;
		XPValue = EnemyEV::Fairy_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Fairy_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fairy_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fairy_Basic_DropChance;
		Speed = EnemyEV::Fairy_Basic_Speed;
		TurnSpeed = EnemyEV::Fairy_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fairy_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Fairy_Basic_Jump;
		CooldownTime = EnemyEV::Fairy_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Fairy_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fairy_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fairy_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fairy_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fairy_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Fairy_Basic_Scale.X, EnemyEV::Fairy_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fairy_Basic_ProjectileScale.X, EnemyEV::Fairy_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fairy_Basic_Tint.R, EnemyEV::Fairy_Basic_Tint.G, EnemyEV::Fairy_Basic_Tint.B));
		MeleeRadius = EnemyEV::Fairy_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Fairy_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Fairy_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fairy_Basic_KnockBack.X, EnemyEV::Fairy_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Fairy::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Fairy::PickAction(const int *weights, int count, const Action *actions)
{
	int total = 0;
	for (int i = 0; i < count; i++)
		total += weights[i];
	int pick = rand() % (total > 0 ? total : 1);
	for (int i = 0; i < count; i++)
	{
		pick -= weights[i];
		if (pick < 0)
		{
			StartAction(actions[i]);
			return;
		}
	}
	StartAction(actions[count - 1]);
}

void EnemyObj_Fairy::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_CHASE:
		/* walkTowardsLS (line 174): fly at the player. */
		ChangeSprite("EnemyFairyGhostMove_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed);
		m_actionTimer = 1.0f;
		break;
	case ACTION_AWAY:
		/* walkAwayLS (line 178): fly away from the player. */
		ChangeSprite("EnemyFairyGhostIdle_Character");
		PlayAnimation(true);
		{
			float dx = X() - m_target->X();
			float dy = Y() - m_target->Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
		}
		CurrentSpeed(Speed);
		m_actionTimer = 0.5f;
		break;
	case ACTION_STOP:
		/* walkStopLS (line 181). */
		ChangeSprite("EnemyFairyGhostIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.5f, 0.75f);
		break;
	case ACTION_FIRE:
		/* fireProjectilesLS (line 184): the windup, the four-way
		 * spread, the recovery. */
		CurrentSpeed(0);
		ChangeSprite("EnemyFairyGhostShoot_Character");
		PlayAnimation("Start", "Windup", false);
		m_actionTimer = AttackDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Fairy::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_CHASE:
		/* Re-aim at the player (ChaseLogicAction). */
		if (m_actionTimer <= 0)
			CurrentSpeed(0);
		else
		{
			float dx = m_target->X() - X();
			float dy = m_target->Y() - Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
		}
		break;
	case ACTION_FIRE:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* ThrowFourProjectiles (line 349): the four directions. */
			static const float angles[4] = { 0, 90, 180, -90 };
			const char *sprite = "GhostProjectile_Sprite";
			for (int i = 0; i < 4; i++)
			{
				float rad = angles[i] * PI / 180.0f;
				m_levelScreen->FireProjectile(sprite, Position(),
				                              Vector2(cosf(rad), sinf(rad)),
				                              ProjectileSpeed, (int) ProjectileDamage,
				                              ProjectileScale, false, this);
			}
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ChangeSprite("EnemyFairyGhostIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
			m_actionTimer = 0.25f;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Fairy::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Fairy::RunBasicLogic()
{
	/* EnemyObj_Fairy.cs line 468.
	 * walkTowardsLS, walkAwayLS, walkStopLS, fireProjectilesLS. */
	static const Action acts[] = { ACTION_CHASE, ACTION_AWAY, ACTION_STOP, ACTION_FIRE };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 50, 10, 0, 40 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_ENGAGE:
		StartAction(ACTION_CHASE);
		break;
	default: /* WANDER */
		StartAction(ACTION_STOP);
		break;
	}
}

} // namespace RogueCastle
