/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Knight.cs. */
#include "EnemyObj_Knight.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cstdlib>
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Knight::EnemyObj_Knight(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySpearKnightIdle_Character", target, difficulty)
{
	Type = EnemyType::Knight;
}

/* The difficulty blocks of InitializeEV only differ by their EV names, so
 * the four cases collapse into one (EnemyObj_Knight.cs line 33). */
void EnemyObj_Knight::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Knight_Miniboss_Name;
		MaxHealth = EnemyEV::Knight_Miniboss_MaxHealth;
		Damage = EnemyEV::Knight_Miniboss_Damage;
		XPValue = EnemyEV::Knight_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Knight_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Knight_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Knight_Miniboss_DropChance;
		Speed = EnemyEV::Knight_Miniboss_Speed;
		TurnSpeed = EnemyEV::Knight_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Knight_Miniboss_ProjectileSpeed;
		CooldownTime = EnemyEV::Knight_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Knight_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Knight_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Knight_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Knight_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Knight_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Knight_Miniboss_Scale.X, EnemyEV::Knight_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Knight_Miniboss_ProjectileScale.X, EnemyEV::Knight_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Knight_Miniboss_Tint.R, EnemyEV::Knight_Miniboss_Tint.G, EnemyEV::Knight_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Knight_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Knight_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Knight_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Knight_Miniboss_KnockBack.X, EnemyEV::Knight_Miniboss_KnockBack.Y);
		/* EnemyObj_Knight.cs line 31. */
		AttackThrustDelay = 0.65f;
		AttackThrustSpeed = 2300;
		AttackThrustDuration = 0.25f;
		break;
	case EXPERT:
		Name = EnemyEV::Knight_Expert_Name;
		MaxHealth = EnemyEV::Knight_Expert_MaxHealth;
		Damage = EnemyEV::Knight_Expert_Damage;
		XPValue = EnemyEV::Knight_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Knight_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Knight_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Knight_Expert_DropChance;
		Speed = EnemyEV::Knight_Expert_Speed;
		TurnSpeed = EnemyEV::Knight_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Knight_Expert_ProjectileSpeed;
		CooldownTime = EnemyEV::Knight_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Knight_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Knight_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Knight_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Knight_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Knight_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Knight_Expert_Scale.X, EnemyEV::Knight_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Knight_Expert_ProjectileScale.X, EnemyEV::Knight_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Knight_Expert_Tint.R, EnemyEV::Knight_Expert_Tint.G, EnemyEV::Knight_Expert_Tint.B));
		MeleeRadius = EnemyEV::Knight_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Knight_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Knight_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Knight_Expert_KnockBack.X, EnemyEV::Knight_Expert_KnockBack.Y);
		/* EnemyObj_Knight.cs line 25. */
		AttackThrustDelay = 0.65f;
		AttackThrustSpeed = 1750;
		AttackThrustDuration = 0.25f;
		break;
	case ADVANCED:
		Name = EnemyEV::Knight_Advanced_Name;
		MaxHealth = EnemyEV::Knight_Advanced_MaxHealth;
		Damage = EnemyEV::Knight_Advanced_Damage;
		XPValue = EnemyEV::Knight_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Knight_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Knight_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Knight_Advanced_DropChance;
		Speed = EnemyEV::Knight_Advanced_Speed;
		TurnSpeed = EnemyEV::Knight_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Knight_Advanced_ProjectileSpeed;
		CooldownTime = EnemyEV::Knight_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Knight_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Knight_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Knight_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Knight_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Knight_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Knight_Advanced_Scale.X, EnemyEV::Knight_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Knight_Advanced_ProjectileScale.X, EnemyEV::Knight_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Knight_Advanced_Tint.R, EnemyEV::Knight_Advanced_Tint.G, EnemyEV::Knight_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Knight_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Knight_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Knight_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Knight_Advanced_KnockBack.X, EnemyEV::Knight_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Knight_Basic_Name;
		MaxHealth = EnemyEV::Knight_Basic_MaxHealth;
		Damage = EnemyEV::Knight_Basic_Damage;
		XPValue = EnemyEV::Knight_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Knight_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Knight_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Knight_Basic_DropChance;
		Speed = EnemyEV::Knight_Basic_Speed;
		TurnSpeed = EnemyEV::Knight_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Knight_Basic_ProjectileSpeed;
		CooldownTime = EnemyEV::Knight_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Knight_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Knight_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Knight_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Knight_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Knight_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Knight_Basic_Scale.X, EnemyEV::Knight_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Knight_Basic_ProjectileScale.X, EnemyEV::Knight_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Knight_Basic_Tint.R, EnemyEV::Knight_Basic_Tint.G, EnemyEV::Knight_Basic_Tint.B));
		MeleeRadius = EnemyEV::Knight_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Knight_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Knight_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Knight_Basic_KnockBack.X, EnemyEV::Knight_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Knight::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Knight::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Knight::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		ChangeSprite("EnemySpearKnightWalk_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                              : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.2f, 1.0f);
		break;
	case ACTION_WALK_STOP:
		ChangeSprite("EnemySpearKnightIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.2f, 1.0f);
		break;
	case ACTION_THRUST:
		/* attackThrustLS: face the player, the tell (windup + delay),
		 * then the charge with AttackStart..End, then recover. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("EnemySpearKnightAttack_Character");
		PlayAnimation("Start", "Windup", false);
		m_actionTimer = 10.0f;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		break;
	case ACTION_THROW_SPEAR:
		/* throwSpearLS (EnemyObj_Knight.cs line 313). */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("EnemySpearKnightAttack2_Character");
		PlayAnimation("Start", "Windup", false);
		m_actionTimer = 10.0f;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		break;
	default:
		m_actionActive = false;
		break;
	}
}

void EnemyObj_Knight::FireSpear(int count, float angleCentre)
{
	/* The spear wave (ThrowThreeProjectiles uses 0/45/-45 fans). */
	if (m_levelScreen == nullptr)
		return;
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	for (int i = 0; i < count; i++)
	{
		float angle = count > 1 ? angleCentre + (i - (count - 1) * 0.5f) * 45.0f : angleCentre;
		float rad = angle * 3.14159265f / 180.0f;
		Vector2 heading(cosf(rad) * facing, sinf(rad));
		m_levelScreen->FireProjectile("EnemySpearKnightWave_Sprite",
		                              Position() + Vector2(30.0f * facing, 0),
		                              heading, ProjectileSpeed, (int) ProjectileDamage,
		                              ProjectileScale, false, this, 0, true);   /* CollidesWithTerrain (the C# ProjectileData) */
	}
}

void EnemyObj_Knight::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_THRUST:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			/* The tell: AttackThrustDelay after the windup. */
			m_actionPhase = 1;
			m_actionTimer = AttackThrustDelay;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			/* MoveDirectionLogicAction: charge in the facing direction. */
			PlayAnimation("AttackStart", "End", false);
			Audio::PlaySound("SpearKnightAttack1");   /* EnemyObj_Knight.cs line 236 */
			CurrentSpeed(AttackThrustSpeed);
			m_actionPhase = 2;
			m_actionTimer = AttackThrustDuration;
		}
		else if (m_actionPhase == 2 && m_actionTimer <= 0)
		{
			/* Stop, back to idle, recover. */
			CurrentSpeed(0);
			ChangeSprite("EnemySpearKnightIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 3;
			m_actionTimer = 0.3f;
		}
		else if (m_actionPhase == 3 && m_actionTimer <= 0)
			m_actionActive = false;
		break;
	case ACTION_THROW_SPEAR:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			m_actionPhase = 1;
			m_actionTimer = AttackProjectileDelay;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			FireSpear(DifficultyType == MINIBOSS ? 3 : 1, 0);
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 2;
		}
		else if (m_actionPhase == 2 && !IsAnimating())
		{
			m_actionPhase = 3;
			m_actionTimer = 0.3f;
		}
		else if (m_actionPhase == 3 && m_actionTimer <= 0)
			m_actionActive = false;
		break;
	default:
		break;
	}
}

void EnemyObj_Knight::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Knight::RunBasicLogic()
{
	/* EnemyObj_Knight.cs line 542.
	 * walkTowardsLS, walkAwayLS, walkStopLS, attackThrustLS, throwSpearLS */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THRUST, ACTION_THROW_SPEAR };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 20, 15, 0, 0, 65 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 20, 20, 10, 0, 50 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 55, 30, 0, 0, 15 };
		PickAction(w, 5, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 0, 0, 100, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_Knight::RunAdvancedLogic()
{
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THRUST, ACTION_THROW_SPEAR };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 15, 10, 0, 60, 15 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 15, 15, 10, 15, 45 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 55, 30, 0, 0, 15 };
		PickAction(w, 5, acts);
		break;
	}
	default:
	{
		static const int w[] = { 0, 0, 100, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_Knight::RunExpertLogic()
{
	/* The expert uses the faster thrust (InitializeOne set its timings). */
	RunAdvancedLogic();
}

void EnemyObj_Knight::RunMinibossLogic()
{
	/* The miniboss uses the faster thrust and a three-spear fan. */
	RunAdvancedLogic();
}

} // namespace RogueCastle
