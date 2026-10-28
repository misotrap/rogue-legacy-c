/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Skeleton.cs. */
#include "EnemyObj_Skeleton.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../ProjectileObj.h"
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

EnemyObj_Skeleton::EnemyObj_Skeleton(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySkeletonIdle_Character", target, difficulty)
{
	/* EnemyObj_Skeleton.cs line 494. */
	Type = EnemyType::Skeleton;
}

void EnemyObj_Skeleton::InitializeEV()
{
	/* EnemyObj_Skeleton.cs line 24 (InitializeEV), per difficulty. */
	switch (DifficultyType)
	{
	case MINIBOSS:
		Name = EnemyEV::Skeleton_Miniboss_Name;
		MaxHealth = EnemyEV::Skeleton_Miniboss_MaxHealth;
		Damage = EnemyEV::Skeleton_Miniboss_Damage;
		XPValue = EnemyEV::Skeleton_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Skeleton_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Skeleton_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Skeleton_Miniboss_DropChance;
		Speed = EnemyEV::Skeleton_Miniboss_Speed;
		TurnSpeed = EnemyEV::Skeleton_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Skeleton_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Skeleton_Miniboss_Jump;
		CooldownTime = EnemyEV::Skeleton_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Skeleton_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Skeleton_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Skeleton_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Skeleton_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Skeleton_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Skeleton_Miniboss_Scale.X, EnemyEV::Skeleton_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Skeleton_Miniboss_ProjectileScale.X, EnemyEV::Skeleton_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Skeleton_Miniboss_Tint.R, EnemyEV::Skeleton_Miniboss_Tint.G, EnemyEV::Skeleton_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Skeleton_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Skeleton_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Skeleton_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Skeleton_Miniboss_KnockBack.X, EnemyEV::Skeleton_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Skeleton_Expert_Name;
		MaxHealth = EnemyEV::Skeleton_Expert_MaxHealth;
		Damage = EnemyEV::Skeleton_Expert_Damage;
		XPValue = EnemyEV::Skeleton_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Skeleton_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Skeleton_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Skeleton_Expert_DropChance;
		Speed = EnemyEV::Skeleton_Expert_Speed;
		TurnSpeed = EnemyEV::Skeleton_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Skeleton_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Skeleton_Expert_Jump;
		CooldownTime = EnemyEV::Skeleton_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Skeleton_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Skeleton_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Skeleton_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Skeleton_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Skeleton_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Skeleton_Expert_Scale.X, EnemyEV::Skeleton_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Skeleton_Expert_ProjectileScale.X, EnemyEV::Skeleton_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Skeleton_Expert_Tint.R, EnemyEV::Skeleton_Expert_Tint.G, EnemyEV::Skeleton_Expert_Tint.B));
		MeleeRadius = EnemyEV::Skeleton_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Skeleton_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Skeleton_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Skeleton_Expert_KnockBack.X, EnemyEV::Skeleton_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Skeleton_Advanced_Name;
		MaxHealth = EnemyEV::Skeleton_Advanced_MaxHealth;
		Damage = EnemyEV::Skeleton_Advanced_Damage;
		XPValue = EnemyEV::Skeleton_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Skeleton_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Skeleton_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Skeleton_Advanced_DropChance;
		Speed = EnemyEV::Skeleton_Advanced_Speed;
		TurnSpeed = EnemyEV::Skeleton_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Skeleton_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Skeleton_Advanced_Jump;
		CooldownTime = EnemyEV::Skeleton_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Skeleton_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Skeleton_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Skeleton_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Skeleton_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Skeleton_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Skeleton_Advanced_Scale.X, EnemyEV::Skeleton_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Skeleton_Advanced_ProjectileScale.X, EnemyEV::Skeleton_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Skeleton_Advanced_Tint.R, EnemyEV::Skeleton_Advanced_Tint.G, EnemyEV::Skeleton_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Skeleton_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Skeleton_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Skeleton_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Skeleton_Advanced_KnockBack.X, EnemyEV::Skeleton_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Skeleton_Basic_Name;
		MaxHealth = EnemyEV::Skeleton_Basic_MaxHealth;
		Damage = EnemyEV::Skeleton_Basic_Damage;
		XPValue = EnemyEV::Skeleton_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Skeleton_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Skeleton_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Skeleton_Basic_DropChance;
		Speed = EnemyEV::Skeleton_Basic_Speed;
		TurnSpeed = EnemyEV::Skeleton_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Skeleton_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Skeleton_Basic_Jump;
		CooldownTime = EnemyEV::Skeleton_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Skeleton_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Skeleton_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Skeleton_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Skeleton_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Skeleton_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Skeleton_Basic_Scale.X, EnemyEV::Skeleton_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Skeleton_Basic_ProjectileScale.X, EnemyEV::Skeleton_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Skeleton_Basic_Tint.R, EnemyEV::Skeleton_Basic_Tint.G, EnemyEV::Skeleton_Basic_Tint.B));
		MeleeRadius = EnemyEV::Skeleton_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Skeleton_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Skeleton_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Skeleton_Basic_KnockBack.X, EnemyEV::Skeleton_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Skeleton::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Skeleton::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS / walkAwayLS (EnemyObj_Skeleton.cs line 187). */
		ChangeSprite("EnemySkeletonWalk_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                              : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.2f, 0.75f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS. */
		ChangeSprite("EnemySkeletonIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.2f, 0.75f);
		break;
	case ACTION_THROW_FAR:
	case ACTION_THROW_HIGH:
		/* throwBoneFarLS / throwBoneHighLS: face the target, windup, fire,
		 * then the throw animation; tagged ATTACK so the cooldown runs. */
		CurrentSpeed(0);
		ChangeSprite("EnemySkeletonAttack_Character");
		PlayAnimation("Start", "Windup", false);
		m_actionTimer = 10.0f;   /* until the windup animation ends */
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		break;
	case ACTION_JUMP_THROW:
		/* jumpBoneFar/HighLS: jump, then throw midair. */
		CurrentSpeed(0);
		ChangeSprite("EnemySkeletonJump_Character");
		PlayAnimation(1, 3, false);
		m_actionTimer = JumpDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		break;
	default:
		m_actionActive = false;
		break;
	}
}

void EnemyObj_Skeleton::ThrowBones(int count, float angleCentre)
{
	/* FireProjectileLogicAction with the bone ProjectileData
	 * (EnemyObj_Skeleton.cs line 185). */
	if (m_levelScreen == nullptr || m_target == nullptr)
		return;
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	for (int i = 0; i < count; i++)
	{
		float angle = angleCentre;
		if (count > 1)
			angle = angleCentre + (i - (count - 1) * 0.5f) * 22.0f;
		float rad = angle * 3.14159265f / 180.0f;
		Vector2 heading(cosf(rad) * facing, sinf(rad));
		Vector2 origin = Position() + Vector2(20.0f * facing, -20.0f);
		Audio::PlaySound("SkeletonAttack1");   /* EnemyObj_Skeleton.cs line 218 */
		m_levelScreen->FireProjectile("BoneProjectile_Sprite", origin, heading,
		                              ProjectileSpeed, (int) ProjectileDamage,
		                              ProjectileScale, true, this);
	}
}

void EnemyObj_Skeleton::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_THROW_FAR:
	case ACTION_THROW_HIGH:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			/* Windup done: the DelayLogicAction(AttackDelay) before the
			 * bone leaves the hand. */
			m_actionPhase = 1;
			m_actionTimer = AttackDelay;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			ThrowBones(DifficultyType == EXPERT || DifficultyType == MINIBOSS ? 3 : 1,
			           m_action == ACTION_THROW_FAR ? -72.0f : -85.0f);
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 2;
		}
		else if (m_actionPhase == 2 && !IsAnimating())
		{
			m_actionActive = false;
			m_actionTimer = RandRange(0.2f, 0.4f);
		}
		break;
	case ACTION_JUMP_THROW:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* JumpLogicAction, then the midair throw. */
			AccelerationY(-JumpHeight);
			ChangeSprite("EnemySkeletonAttack_Character");
			PlayAnimation("Start", "Windup", false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ThrowBones(DifficultyType == EXPERT ? 3 : 1, -72.0f);
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 2;
		}
		else if (m_actionPhase == 2 && !IsAnimating())
			m_actionActive = false;
		break;
	default:
		break;
	}
}

void EnemyObj_Skeleton::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Skeleton::RunBasicLogic()
{
	/* EnemyObj_Skeleton.cs line 405: the probability tables.
	 * walkTowardsLS, walkAwayLS, walkStopLS, throwBoneFarLS, throwBoneHighLS */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THROW_FAR, ACTION_THROW_HIGH };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 10, 10, 0, 30, 50 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 10, 10, 0, 40, 40 };
		PickAction(w, 5, acts);
		break;
	}
	default: /* ENGAGE / WANDER */
	{
		static const int w[] = { 40, 40, 20, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_Skeleton::RunAdvancedLogic()
{
	/* + jumpBoneFarLS, jumpBoneHighLS. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THROW_FAR, ACTION_THROW_HIGH, ACTION_JUMP_THROW };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 10, 10, 0, 15, 15, 25, 25 };
		PickAction(w, 7, acts);
		break;
	}
	default:
	{
		static const int w[] = { 40, 40, 20, 0, 0, 0, 0 };
		PickAction(w, 7, acts);
		break;
	}
	}
}

void EnemyObj_Skeleton::RunExpertLogic()
{
	/* throwBoneExpertLS (three bones) + jumpBoneExpertLS. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THROW_FAR, ACTION_JUMP_THROW };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 15, 15, 0, 35, 35 };
		PickAction(w, 5, acts);
		break;
	}
	default:
	{
		static const int w[] = { 35, 35, 0, 0, 15 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_Skeleton::RunMinibossLogic()
{
	/* The miniboss throws a fan of bones (throwBoneMiniBossLS). */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_THROW_FAR, ACTION_THROW_HIGH };
	switch (State)
	{
	default:
	{
		static const int w[] = { 0, 0, 10, 90, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

} // namespace RogueCastle
