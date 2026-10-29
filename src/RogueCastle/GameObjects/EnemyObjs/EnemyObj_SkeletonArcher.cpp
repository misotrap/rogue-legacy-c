/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SkeletonArcher.cs. */
#include "EnemyObj_SkeletonArcher.h"
#include "../ProjectileObj.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include <cstdlib>
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_SkeletonArcher::EnemyObj_SkeletonArcher(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySkeletonArcherIdle_Character", target, difficulty)
{
	Type = EnemyType::SkeletonArcher;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_SkeletonArcher.cs line 22), so they collapse into one. */
void EnemyObj_SkeletonArcher::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::SkeletonArcher_Miniboss_Name;
		MaxHealth = EnemyEV::SkeletonArcher_Miniboss_MaxHealth;
		Damage = EnemyEV::SkeletonArcher_Miniboss_Damage;
		XPValue = EnemyEV::SkeletonArcher_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::SkeletonArcher_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SkeletonArcher_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::SkeletonArcher_Miniboss_DropChance;
		Speed = EnemyEV::SkeletonArcher_Miniboss_Speed;
		TurnSpeed = EnemyEV::SkeletonArcher_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::SkeletonArcher_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::SkeletonArcher_Miniboss_Jump;
		CooldownTime = EnemyEV::SkeletonArcher_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::SkeletonArcher_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SkeletonArcher_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SkeletonArcher_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SkeletonArcher_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SkeletonArcher_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::SkeletonArcher_Miniboss_Scale.X, EnemyEV::SkeletonArcher_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SkeletonArcher_Miniboss_ProjectileScale.X, EnemyEV::SkeletonArcher_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SkeletonArcher_Miniboss_Tint.R, EnemyEV::SkeletonArcher_Miniboss_Tint.G, EnemyEV::SkeletonArcher_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::SkeletonArcher_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::SkeletonArcher_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::SkeletonArcher_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::SkeletonArcher_Miniboss_KnockBack.X, EnemyEV::SkeletonArcher_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::SkeletonArcher_Expert_Name;
		MaxHealth = EnemyEV::SkeletonArcher_Expert_MaxHealth;
		Damage = EnemyEV::SkeletonArcher_Expert_Damage;
		XPValue = EnemyEV::SkeletonArcher_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::SkeletonArcher_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SkeletonArcher_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::SkeletonArcher_Expert_DropChance;
		Speed = EnemyEV::SkeletonArcher_Expert_Speed;
		TurnSpeed = EnemyEV::SkeletonArcher_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::SkeletonArcher_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::SkeletonArcher_Expert_Jump;
		CooldownTime = EnemyEV::SkeletonArcher_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::SkeletonArcher_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SkeletonArcher_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SkeletonArcher_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SkeletonArcher_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SkeletonArcher_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::SkeletonArcher_Expert_Scale.X, EnemyEV::SkeletonArcher_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SkeletonArcher_Expert_ProjectileScale.X, EnemyEV::SkeletonArcher_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SkeletonArcher_Expert_Tint.R, EnemyEV::SkeletonArcher_Expert_Tint.G, EnemyEV::SkeletonArcher_Expert_Tint.B));
		MeleeRadius = EnemyEV::SkeletonArcher_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::SkeletonArcher_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::SkeletonArcher_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::SkeletonArcher_Expert_KnockBack.X, EnemyEV::SkeletonArcher_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::SkeletonArcher_Advanced_Name;
		MaxHealth = EnemyEV::SkeletonArcher_Advanced_MaxHealth;
		Damage = EnemyEV::SkeletonArcher_Advanced_Damage;
		XPValue = EnemyEV::SkeletonArcher_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::SkeletonArcher_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SkeletonArcher_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::SkeletonArcher_Advanced_DropChance;
		Speed = EnemyEV::SkeletonArcher_Advanced_Speed;
		TurnSpeed = EnemyEV::SkeletonArcher_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::SkeletonArcher_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::SkeletonArcher_Advanced_Jump;
		CooldownTime = EnemyEV::SkeletonArcher_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::SkeletonArcher_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SkeletonArcher_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SkeletonArcher_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SkeletonArcher_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SkeletonArcher_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::SkeletonArcher_Advanced_Scale.X, EnemyEV::SkeletonArcher_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SkeletonArcher_Advanced_ProjectileScale.X, EnemyEV::SkeletonArcher_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SkeletonArcher_Advanced_Tint.R, EnemyEV::SkeletonArcher_Advanced_Tint.G, EnemyEV::SkeletonArcher_Advanced_Tint.B));
		MeleeRadius = EnemyEV::SkeletonArcher_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::SkeletonArcher_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::SkeletonArcher_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::SkeletonArcher_Advanced_KnockBack.X, EnemyEV::SkeletonArcher_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::SkeletonArcher_Basic_Name;
		MaxHealth = EnemyEV::SkeletonArcher_Basic_MaxHealth;
		Damage = EnemyEV::SkeletonArcher_Basic_Damage;
		XPValue = EnemyEV::SkeletonArcher_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::SkeletonArcher_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SkeletonArcher_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::SkeletonArcher_Basic_DropChance;
		Speed = EnemyEV::SkeletonArcher_Basic_Speed;
		TurnSpeed = EnemyEV::SkeletonArcher_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::SkeletonArcher_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::SkeletonArcher_Basic_Jump;
		CooldownTime = EnemyEV::SkeletonArcher_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::SkeletonArcher_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SkeletonArcher_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SkeletonArcher_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SkeletonArcher_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SkeletonArcher_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::SkeletonArcher_Basic_Scale.X, EnemyEV::SkeletonArcher_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SkeletonArcher_Basic_ProjectileScale.X, EnemyEV::SkeletonArcher_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SkeletonArcher_Basic_Tint.R, EnemyEV::SkeletonArcher_Basic_Tint.G, EnemyEV::SkeletonArcher_Basic_Tint.B));
		MeleeRadius = EnemyEV::SkeletonArcher_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::SkeletonArcher_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::SkeletonArcher_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::SkeletonArcher_Basic_KnockBack.X, EnemyEV::SkeletonArcher_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_SkeletonArcher::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_SkeletonArcher::FireArrow(float angleDeg)
{
	/* The ProjectileData (line 193): a weighted arrow at a fixed angle
	 * (negative degrees = upward). */
	float rad = angleDeg * PI / 180.0f;
	ProjectileObj *arrow = m_levelScreen->FireProjectile("ArrowProjectile_Sprite", Position(),
	                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
	                              (int) ProjectileDamage, ProjectileScale, true, this, 0, true);   /* CollidesWithTerrain (the C# ProjectileData) */
	arrow->RotationSpeed = 0;
	arrow->FollowArc = true;
}

/* The arrow fans per difficulty and shot (the C# sets projData.Angle
 * before each FireProjectileLogicAction). */
static float ArrowAngle(int actionKind, int shotIndex, int difficulty)
{
	/* actionKind: 0 = straight, 1 = high arch, 2 = arch. */
	static const float angles[3][4][3] = {
		/* straight: basic, advanced, expert */
		{ { -20, 0, 0 }, { -20, -50, 0 }, { -35, -15, -55 } },
		/* high arch */
		{ { -70, 0, 0 }, { -55, -85, 0 }, { -90, -85, -95 } },
		/* arch */
		{ { -50, 0, 0 }, { -50, -75, 0 }, { -60, -75, -45 } },
	};
	int d = difficulty == 0 ? 0 : (difficulty == 1 ? 1 : 2);
	return angles[actionKind][d][shotIndex];
}

void EnemyObj_SkeletonArcher::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_SkeletonArcher::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_STOP:
		/* walkStopLS (line 147). */
		ChangeSprite("EnemySkeletonArcherIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.2f, 0.75f);
		break;
	case ACTION_FIRE_STRAIGHT:
	case ACTION_FIRE_HIGH_ARCH:
	case ACTION_FIRE_ARCH:
		/* fireStraight/fireHighArch/fireArch (lines 166, 330, 267):
		 * the windup, the arrows, the recovery. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		Audio::PlaySound("SkeletonArcher_Load");   /* EnemyObj_SkeletonArcher.cs line 171 */
		ChangeSprite("EnemySkeletonArcherAttack_Character");
		PlayAnimation("Start", "BeginAttack", false);
		m_actionTimer = m_fireDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		switch (DifficultyType)
		{
		case EXPERT: m_shotsLeft = 3; break;
		case ADVANCED: m_shotsLeft = 2; break;
		default: m_shotsLeft = 1; break;
		}
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_SkeletonArcher::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_FIRE_STRAIGHT:
	case ACTION_FIRE_HIGH_ARCH:
	case ACTION_FIRE_ARCH:
	{
		int kind = m_action == ACTION_FIRE_STRAIGHT ? 0
		         : (m_action == ACTION_FIRE_HIGH_ARCH ? 1 : 2);
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The arrows all leave together (the C# fires them back to
			 * back with no delay between). */
			int diff = DifficultyType == BASIC ? 0 : (DifficultyType == ADVANCED ? 1 : 2);
			Audio::PlaySound("SkeletonArcher_Attack_01", "SkeletonArcher_Attack_02", "SkeletonArcher_Attack_03");   /* EnemyObj_SkeletonArcher.cs line 175 */
			for (int i = 0; i < m_shotsLeft; i++)
				FireArrow(ArrowAngle(kind, i, diff));
			PlayAnimation("Attack", "EndAttack", false);
			m_actionPhase = 1;
			m_actionTimer = 1.0f;   /* the two 0.5 s delays */
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0 && !IsAnimating())
		{
			ChangeSprite("EnemySkeletonArcherIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	}
	default:
		break;
	}
}

void EnemyObj_SkeletonArcher::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_SkeletonArcher::RunBasicLogic()
{
	/* EnemyObj_SkeletonArcher.cs line 408: the LB is
	 * {walkStop, fireStraight, fireHighArch, fireArch}; the weights
	 * follow the state (the arrows fly straightest when the player is
	 * far, highest up close). */
	static const Action acts[] = { ACTION_STOP, ACTION_FIRE_STRAIGHT, ACTION_FIRE_HIGH_ARCH, ACTION_FIRE_ARCH };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 20, 50, 15, 15 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 20, 15, 50, 15 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 20, 15, 15, 50 };
		PickAction(w, 4, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 100, 0, 0, 0 };
		PickAction(w, 4, acts);
		break;
	}
	}
}

} // namespace RogueCastle
