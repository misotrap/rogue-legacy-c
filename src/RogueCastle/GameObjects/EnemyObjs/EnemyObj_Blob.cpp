/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Blob.cs. */
#include "EnemyObj_Blob.h"
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

EnemyObj_Blob::EnemyObj_Blob(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyBlobIdle_Character", target, difficulty)
{
	Type = EnemyType::Blob;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Blob.cs line 47), so they collapse into one. */
void EnemyObj_Blob::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Blob_Miniboss_Name;
		MaxHealth = EnemyEV::Blob_Miniboss_MaxHealth;
		Damage = EnemyEV::Blob_Miniboss_Damage;
		XPValue = EnemyEV::Blob_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Blob_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Blob_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Blob_Miniboss_DropChance;
		Speed = EnemyEV::Blob_Miniboss_Speed;
		TurnSpeed = EnemyEV::Blob_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Blob_Miniboss_ProjectileSpeed;
		CooldownTime = EnemyEV::Blob_Miniboss_Cooldown;
		JumpHeight = EnemyEV::Blob_Miniboss_Jump;
		AnimationDelay(1 / EnemyEV::Blob_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Blob_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Blob_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Blob_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Blob_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Blob_Miniboss_Scale.X, EnemyEV::Blob_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Blob_Miniboss_ProjectileScale.X, EnemyEV::Blob_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Blob_Miniboss_Tint.R, EnemyEV::Blob_Miniboss_Tint.G, EnemyEV::Blob_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Blob_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Blob_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Blob_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Blob_Miniboss_KnockBack.X, EnemyEV::Blob_Miniboss_KnockBack.Y);
		m_resetSpriteName = "EnemyBlobBossIdle_Character";
		break;
	case EXPERT:
		Name = EnemyEV::Blob_Expert_Name;
		MaxHealth = EnemyEV::Blob_Expert_MaxHealth;
		Damage = EnemyEV::Blob_Expert_Damage;
		XPValue = EnemyEV::Blob_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Blob_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Blob_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Blob_Expert_DropChance;
		Speed = EnemyEV::Blob_Expert_Speed;
		TurnSpeed = EnemyEV::Blob_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Blob_Expert_ProjectileSpeed;
		CooldownTime = EnemyEV::Blob_Expert_Cooldown;
		JumpHeight = EnemyEV::Blob_Expert_Jump;
		AnimationDelay(1 / EnemyEV::Blob_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Blob_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Blob_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Blob_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Blob_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Blob_Expert_Scale.X, EnemyEV::Blob_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Blob_Expert_ProjectileScale.X, EnemyEV::Blob_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Blob_Expert_Tint.R, EnemyEV::Blob_Expert_Tint.G, EnemyEV::Blob_Expert_Tint.B));
		MeleeRadius = EnemyEV::Blob_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Blob_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Blob_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Blob_Expert_KnockBack.X, EnemyEV::Blob_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Blob_Advanced_Name;
		MaxHealth = EnemyEV::Blob_Advanced_MaxHealth;
		Damage = EnemyEV::Blob_Advanced_Damage;
		XPValue = EnemyEV::Blob_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Blob_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Blob_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Blob_Advanced_DropChance;
		Speed = EnemyEV::Blob_Advanced_Speed;
		TurnSpeed = EnemyEV::Blob_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Blob_Advanced_ProjectileSpeed;
		CooldownTime = EnemyEV::Blob_Advanced_Cooldown;
		JumpHeight = EnemyEV::Blob_Advanced_Jump;
		AnimationDelay(1 / EnemyEV::Blob_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Blob_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Blob_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Blob_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Blob_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Blob_Advanced_Scale.X, EnemyEV::Blob_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Blob_Advanced_ProjectileScale.X, EnemyEV::Blob_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Blob_Advanced_Tint.R, EnemyEV::Blob_Advanced_Tint.G, EnemyEV::Blob_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Blob_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Blob_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Blob_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Blob_Advanced_KnockBack.X, EnemyEV::Blob_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Blob_Basic_Name;
		MaxHealth = EnemyEV::Blob_Basic_MaxHealth;
		Damage = EnemyEV::Blob_Basic_Damage;
		XPValue = EnemyEV::Blob_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Blob_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Blob_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Blob_Basic_DropChance;
		Speed = EnemyEV::Blob_Basic_Speed;
		TurnSpeed = EnemyEV::Blob_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Blob_Basic_ProjectileSpeed;
		CooldownTime = EnemyEV::Blob_Basic_Cooldown;
		JumpHeight = EnemyEV::Blob_Basic_Jump;
		AnimationDelay(1 / EnemyEV::Blob_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Blob_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Blob_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Blob_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Blob_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Blob_Basic_Scale.X, EnemyEV::Blob_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Blob_Basic_ProjectileScale.X, EnemyEV::Blob_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Blob_Basic_Tint.R, EnemyEV::Blob_Basic_Tint.G, EnemyEV::Blob_Basic_Tint.B));
		MeleeRadius = EnemyEV::Blob_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Blob_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Blob_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Blob_Basic_KnockBack.X, EnemyEV::Blob_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Blob::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Blob::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Blob::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
		/* walkTowardsLS (line 263). */
		HeadingX(m_target->X() >= X() ? 1 : -1);
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(1.1f, 1.9f);
		break;
	case ACTION_WALK_AWAY:
		/* walkAwayLS (line 268). */
		HeadingX(m_target->X() >= X() ? -1 : 1);
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(1.0f, 1.5f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 273). */
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.5f, 0.9f);
		break;
	case ACTION_JUMP:
		/* jumpLS/jumpAdvancedLS/jumpExpertLS (lines 277, 317, 341): the
		 * crouch, then the hop towards the player, then land. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite(DifficultyType == MINIBOSS ? "EnemyBlobBossJump_Character"
		                                       : "EnemyBlobJump_Character");
		PlayAnimation("Start", "BeforeJump", false);
		m_actionTimer = JumpDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 4;
		break;
	default:
		break;
	}
}

void EnemyObj_Blob::UpdateAction()
{
	bool boss = DifficultyType == MINIBOSS;
	const char *air = boss ? "EnemyBlobBossAir_Character" : "EnemyBlobAir_Character";
	const char *jump = boss ? "EnemyBlobBossJump_Character" : "EnemyBlobJump_Character";
	const char *idle = boss ? "EnemyBlobBossIdle_Character" : "EnemyBlobIdle_Character";

	switch (m_action)
	{
	case ACTION_JUMP:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* Launch: 6.75x speed towards the player (line 284), the jump
			 * impulse, and for Advanced+ the damage shield (line 326). */
			CurrentSpeed(Speed * 6.75f);
			if (DifficultyType == ADVANCED || DifficultyType == EXPERT)
				m_levelScreen->FireProjectile("SpellDamageShield_Sprite",
				                              Position() + Vector2(0, 10), Vector2(1, 0),
				                              ProjectileSpeed, (int) ProjectileDamage,
				                              ProjectileScale, false, this,
				                              5.0f);   /* ExpertBlobProjectileDuration (line 30) */
			Audio::PlaySound("Blob_Jump");   /* EnemyObj_Blob.cs line 287 */
			AccelerationY(-JumpHeight);
			ChangeSprite(air);
			PlayAnimation(true);
			m_actionPhase = 1;
			/* Line 289: a delay before the ground check, or the blob
			 * would be flagged as landed on its first air frame. */
			m_actionTimer = 0.2f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0 && m_isTouchingGround)
		{
			/* GroundCheckLogicAction: wait until the blob lands. */
			Audio::PlaySound("Blob_Land");   /* EnemyObj_Blob.cs line 291 */
			CurrentSpeed(Speed);
			ChangeSprite(jump);
			PlayAnimation("Start", "Jump", false);
			m_actionPhase = 2;
		}
		else if (m_actionPhase == 2 && !IsAnimating())
		{
			ChangeSprite(idle);
			PlayAnimation(true);
			m_actionPhase = 3;
			m_actionTimer = 0.2f;
		}
		else if (m_actionPhase == 3 && m_actionTimer <= 0)
			m_actionPhase = 4;
		break;
	default:
		break;
	}
}

void EnemyObj_Blob::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* EnemyObj_Blob.cs line 677: keep the air speed when the physics
	 * landing zeroes it, and go back to idle when landing between
	 * actions. */
	const char *air = DifficultyType == MINIBOSS ? "EnemyBlobBossAir_Character" : "EnemyBlobAir_Character";
	const char *idle = DifficultyType == MINIBOSS ? "EnemyBlobBossIdle_Character" : "EnemyBlobIdle_Character";
	if (!m_isTouchingGround && CurrentSpeed() == 0 && SpriteName() == air)
		CurrentSpeed(Speed);
	if (!m_actionActive && m_isTouchingGround && SpriteName() != idle)
	{
		ChangeSprite(idle);
		PlayAnimation(true);
	}
	UpdateAction();
}

void EnemyObj_Blob::RunBasicLogic()
{
	/* EnemyObj_Blob.cs line 444: only act when grounded.
	 * walkTowardsLS, walkAwayLS, walkStopLS, jumpLS. */
	if (!m_isTouchingGround)
		return;
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP, ACTION_JUMP };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 45, 0, 0, 55 };
		PickAction(w, 4, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 10, 10, 75, 5 };
		PickAction(w, 4, acts);
		break;
	}
	}
}

void EnemyObj_Blob::RunAdvancedLogic()
{
	/* Line 464: the same sets; the Advanced jump also drops the shield. */
	RunBasicLogic();
}

void EnemyObj_Blob::RunExpertLogic()
{
	/* Line 484: the same sets; the Expert jump also drops the shield. */
	RunBasicLogic();
}

void EnemyObj_Blob::RunMinibossLogic()
{
	/* Line 504: Alexander hops the same way (the boss sprites); the
	 * splitting is not ported yet. */
	RunBasicLogic();
}

} // namespace RogueCastle
