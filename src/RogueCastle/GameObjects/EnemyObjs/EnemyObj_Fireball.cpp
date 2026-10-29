/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Fireball.cs. */
#include "EnemyObj_Fireball.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Fireball::EnemyObj_Fireball(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyGhostIdle_Character", target, difficulty)
{
	Type = EnemyType::Fireball;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Fireball.cs line 37), so they collapse into one. */
void EnemyObj_Fireball::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Fireball_Miniboss_Name;
		MaxHealth = EnemyEV::Fireball_Miniboss_MaxHealth;
		Damage = EnemyEV::Fireball_Miniboss_Damage;
		XPValue = EnemyEV::Fireball_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Fireball_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fireball_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fireball_Miniboss_DropChance;
		Speed = EnemyEV::Fireball_Miniboss_Speed;
		TurnSpeed = EnemyEV::Fireball_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fireball_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Fireball_Miniboss_Jump;
		CooldownTime = EnemyEV::Fireball_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Fireball_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fireball_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fireball_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fireball_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fireball_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Fireball_Miniboss_Scale.X, EnemyEV::Fireball_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fireball_Miniboss_ProjectileScale.X, EnemyEV::Fireball_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fireball_Miniboss_Tint.R, EnemyEV::Fireball_Miniboss_Tint.G, EnemyEV::Fireball_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Fireball_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Fireball_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Fireball_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fireball_Miniboss_KnockBack.X, EnemyEV::Fireball_Miniboss_KnockBack.Y);
		m_resetSpriteName = "EnemyGhostBossIdle_Character";   /* line 158 */
		break;
	case EXPERT:
		Name = EnemyEV::Fireball_Expert_Name;
		MaxHealth = EnemyEV::Fireball_Expert_MaxHealth;
		Damage = EnemyEV::Fireball_Expert_Damage;
		XPValue = EnemyEV::Fireball_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Fireball_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fireball_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fireball_Expert_DropChance;
		Speed = EnemyEV::Fireball_Expert_Speed;
		TurnSpeed = EnemyEV::Fireball_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fireball_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Fireball_Expert_Jump;
		CooldownTime = EnemyEV::Fireball_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Fireball_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fireball_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fireball_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fireball_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fireball_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Fireball_Expert_Scale.X, EnemyEV::Fireball_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fireball_Expert_ProjectileScale.X, EnemyEV::Fireball_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fireball_Expert_Tint.R, EnemyEV::Fireball_Expert_Tint.G, EnemyEV::Fireball_Expert_Tint.B));
		MeleeRadius = EnemyEV::Fireball_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Fireball_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Fireball_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fireball_Expert_KnockBack.X, EnemyEV::Fireball_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Fireball_Advanced_Name;
		MaxHealth = EnemyEV::Fireball_Advanced_MaxHealth;
		Damage = EnemyEV::Fireball_Advanced_Damage;
		XPValue = EnemyEV::Fireball_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Fireball_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fireball_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fireball_Advanced_DropChance;
		Speed = EnemyEV::Fireball_Advanced_Speed;
		TurnSpeed = EnemyEV::Fireball_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fireball_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Fireball_Advanced_Jump;
		CooldownTime = EnemyEV::Fireball_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Fireball_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fireball_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fireball_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fireball_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fireball_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Fireball_Advanced_Scale.X, EnemyEV::Fireball_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fireball_Advanced_ProjectileScale.X, EnemyEV::Fireball_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fireball_Advanced_Tint.R, EnemyEV::Fireball_Advanced_Tint.G, EnemyEV::Fireball_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Fireball_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Fireball_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Fireball_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fireball_Advanced_KnockBack.X, EnemyEV::Fireball_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Fireball_Basic_Name;
		MaxHealth = EnemyEV::Fireball_Basic_MaxHealth;
		Damage = EnemyEV::Fireball_Basic_Damage;
		XPValue = EnemyEV::Fireball_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Fireball_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Fireball_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Fireball_Basic_DropChance;
		Speed = EnemyEV::Fireball_Basic_Speed;
		TurnSpeed = EnemyEV::Fireball_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Fireball_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Fireball_Basic_Jump;
		CooldownTime = EnemyEV::Fireball_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Fireball_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Fireball_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Fireball_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Fireball_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Fireball_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Fireball_Basic_Scale.X, EnemyEV::Fireball_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Fireball_Basic_ProjectileScale.X, EnemyEV::Fireball_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Fireball_Basic_Tint.R, EnemyEV::Fireball_Basic_Tint.G, EnemyEV::Fireball_Basic_Tint.B));
		MeleeRadius = EnemyEV::Fireball_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Fireball_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Fireball_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Fireball_Basic_KnockBack.X, EnemyEV::Fireball_Basic_KnockBack.Y);
		break;
	}
	DashDelay = 0.75f;   /* line 46 */
	DashDuration = 0.5f;
	DashSpeed = 900;
}

void EnemyObj_Fireball::InitializeEV()
{
	m_cooldownWeights[0] = 100; m_cooldownWeights[1] = 0; m_cooldownWeights[2] = 0;
	InitializeOne(DifficultyType);
}

void EnemyObj_Fireball::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Fireball::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_CHASE:
		/* walkTowardsLS (line 171): chase the player (flying, so the
		 * heading is a full 2D direction). */
		ChangeSprite("EnemyGhostChase_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed);
		m_actionTimer = 1.0f;   /* the chase re-aims every cycle */
		break;
	case ACTION_STOP:
		/* walkStopLS (line 177). */
		ChangeSprite("EnemyGhostIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(1.0f, 2.0f);
		break;
	case ACTION_DASH:
		/* dashTowardsLS (line 185): the prep, the dash at the player. */
		CurrentSpeed(0);
		ChangeSprite("EnemyGhostDashPrep_Character");
		PlayAnimation(true);
		m_actionTimer = DashDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Fireball::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_CHASE:
		/* Re-aim at the player every cycle (ChaseLogicAction). */
		if (m_actionTimer <= 0)
		{
			float dx = m_target->X() - X();
			float dy = m_target->Y() - Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
		}
		break;
	case ACTION_DASH:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* TurnToPlayer, then dash (line 237-239). */
			float dx = m_target->X() - X();
			float dy = m_target->Y() - Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
			ChangeSprite("EnemyGhostDash_Character");
			PlayAnimation(true);
			CurrentSpeed(DashSpeed);
			m_actionPhase = 1;
			m_actionTimer = DashDuration;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyGhostIdle_Character");
			PlayAnimation(true);
			CurrentSpeed(0);
			m_actionPhase = 2;
			m_actionTimer = 0.75f;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Fireball::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Fireball::RunBasicLogic()
{
	/* EnemyObj_Fireball.cs line 401: chase when engaged, stop when
	 * wandering (the dash sets are for the higher difficulties). */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		StartAction(ACTION_CHASE);
		break;
	default:
		StartAction(ACTION_STOP);
		break;
	}
}

void EnemyObj_Fireball::RunAdvancedLogic()
{
	/* Line 417: the dash when the player is close (the four-projectile
	 * dash of the original is not ported). */
	static const Action acts[] = { ACTION_CHASE, ACTION_DASH };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 40, 60 };
		PickAction(w, 2, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		StartAction(ACTION_CHASE);
		break;
	default:
		StartAction(ACTION_STOP);
		break;
	}
}

} // namespace RogueCastle

void RogueCastle::EnemyObj_Fireball::StartCooldownMove(int move)
{
	static const Action moves[3] = { ACTION_CHASE, ACTION_CHASE, ACTION_STOP };
	StartAction(moves[move]);
}
