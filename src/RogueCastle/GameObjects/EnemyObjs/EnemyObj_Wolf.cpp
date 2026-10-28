/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Wolf.cs. */
#include "EnemyObj_Wolf.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_Wolf::EnemyObj_Wolf(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyWargIdle_Character", target, difficulty)
{
	Type = EnemyType::Wolf;
	m_startDelayCounter = m_startDelay;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Wolf.cs line 23), so they collapse into one. */
void EnemyObj_Wolf::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Wolf_Miniboss_Name;
		MaxHealth = EnemyEV::Wolf_Miniboss_MaxHealth;
		Damage = EnemyEV::Wolf_Miniboss_Damage;
		XPValue = EnemyEV::Wolf_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Wolf_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Wolf_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Wolf_Miniboss_DropChance;
		Speed = EnemyEV::Wolf_Miniboss_Speed;
		TurnSpeed = EnemyEV::Wolf_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Wolf_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Wolf_Miniboss_Jump;
		CooldownTime = EnemyEV::Wolf_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Wolf_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Wolf_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Wolf_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Wolf_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Wolf_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Wolf_Miniboss_Scale.X, EnemyEV::Wolf_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Wolf_Miniboss_ProjectileScale.X, EnemyEV::Wolf_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Wolf_Miniboss_Tint.R, EnemyEV::Wolf_Miniboss_Tint.G, EnemyEV::Wolf_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Wolf_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Wolf_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Wolf_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Wolf_Miniboss_KnockBack.X, EnemyEV::Wolf_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Wolf_Expert_Name;
		MaxHealth = EnemyEV::Wolf_Expert_MaxHealth;
		Damage = EnemyEV::Wolf_Expert_Damage;
		XPValue = EnemyEV::Wolf_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Wolf_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Wolf_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Wolf_Expert_DropChance;
		Speed = EnemyEV::Wolf_Expert_Speed;
		TurnSpeed = EnemyEV::Wolf_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Wolf_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Wolf_Expert_Jump;
		CooldownTime = EnemyEV::Wolf_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Wolf_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Wolf_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Wolf_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Wolf_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Wolf_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Wolf_Expert_Scale.X, EnemyEV::Wolf_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Wolf_Expert_ProjectileScale.X, EnemyEV::Wolf_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Wolf_Expert_Tint.R, EnemyEV::Wolf_Expert_Tint.G, EnemyEV::Wolf_Expert_Tint.B));
		MeleeRadius = EnemyEV::Wolf_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Wolf_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Wolf_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Wolf_Expert_KnockBack.X, EnemyEV::Wolf_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Wolf_Advanced_Name;
		MaxHealth = EnemyEV::Wolf_Advanced_MaxHealth;
		Damage = EnemyEV::Wolf_Advanced_Damage;
		XPValue = EnemyEV::Wolf_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Wolf_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Wolf_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Wolf_Advanced_DropChance;
		Speed = EnemyEV::Wolf_Advanced_Speed;
		TurnSpeed = EnemyEV::Wolf_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Wolf_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Wolf_Advanced_Jump;
		CooldownTime = EnemyEV::Wolf_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Wolf_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Wolf_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Wolf_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Wolf_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Wolf_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Wolf_Advanced_Scale.X, EnemyEV::Wolf_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Wolf_Advanced_ProjectileScale.X, EnemyEV::Wolf_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Wolf_Advanced_Tint.R, EnemyEV::Wolf_Advanced_Tint.G, EnemyEV::Wolf_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Wolf_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Wolf_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Wolf_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Wolf_Advanced_KnockBack.X, EnemyEV::Wolf_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Wolf_Basic_Name;
		MaxHealth = EnemyEV::Wolf_Basic_MaxHealth;
		Damage = EnemyEV::Wolf_Basic_Damage;
		XPValue = EnemyEV::Wolf_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Wolf_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Wolf_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Wolf_Basic_DropChance;
		Speed = EnemyEV::Wolf_Basic_Speed;
		TurnSpeed = EnemyEV::Wolf_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Wolf_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Wolf_Basic_Jump;
		CooldownTime = EnemyEV::Wolf_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Wolf_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Wolf_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Wolf_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Wolf_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Wolf_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Wolf_Basic_Scale.X, EnemyEV::Wolf_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Wolf_Basic_ProjectileScale.X, EnemyEV::Wolf_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Wolf_Basic_Tint.R, EnemyEV::Wolf_Basic_Tint.G, EnemyEV::Wolf_Basic_Tint.B));
		MeleeRadius = EnemyEV::Wolf_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Wolf_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Wolf_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Wolf_Basic_KnockBack.X, EnemyEV::Wolf_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Wolf::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Wolf::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_WALK:
		/* walkTowardsLS (line 151): chase for a second. */
		ChangeSprite("EnemyWargRun_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		CurrentSpeed(Speed);
		Chasing = true;
		m_actionTimer = 1.0f;
		break;
	case ACTION_STOP:
		/* stopWalkLS (line 158). */
		ChangeSprite("EnemyWargIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		Chasing = false;
		m_actionTimer = 1.0f;
		break;
	case ACTION_POUNCE:
		/* jumpLS (line 165): the crouch, the leap, the landing. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("EnemyWargPounce_Character");
		PlayAnimation(true);
		m_actionTimer = PounceDelay;
		m_endPhase = 3;
		break;
	case ACTION_HIT:
		/* wolfHit (line 175): the hurt sprite interrupt. */
		ChangeSprite("EnemyWargHit_Character");
		PlayAnimation(true);
		m_actionTimer = 0.2f;
		m_endPhase = 1;
		break;
	default:
		break;
	}
}

void EnemyObj_Wolf::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_POUNCE:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The leap (line 213): jump sprite, move in the facing
			 * direction, the jump impulse, the attack frames. */
			ChangeSprite("EnemyWargJump_Character");
			PlayAnimation("Attack", "End", false);
			CurrentSpeed(Speed);
			AccelerationY(-JumpHeight);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && m_isTouchingGround && !IsAnimating())
		{
			/* GroundCheckLogicAction: wait for the landing. */
			CurrentSpeed(0);
			ChangeSprite("EnemyWargIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
			m_actionTimer = PounceLandDelay;
		}
		else if (m_actionPhase == 2 && m_actionTimer <= 0)
			m_actionPhase = 3;
		break;
	case ACTION_HIT:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyWargIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 1;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Wolf::UpdateEnemy(GameTime *gameTime)
{
	if (m_startDelayCounter > 0)
		m_startDelayCounter -= (float) gameTime->ElapsedGameTime;
	/* Line 292: keep the air speed through the landing, and go back to
	 * idle when standing between actions. */
	if (!m_isTouchingGround && CurrentSpeed() == 0 && SpriteName() == "EnemyWargJump_Character")
		CurrentSpeed(Speed);
	if (m_isTouchingGround && CurrentSpeed() == 0 && !IsAnimating() && m_action != ACTION_POUNCE)
	{
		ChangeSprite("EnemyWargIdle_Character");
		PlayAnimation(true);
	}
	UpdateAction();
}

void EnemyObj_Wolf::RunBasicLogic()
{
	/* EnemyObj_Wolf.cs line 227: nothing until the start delay is over. */
	if (m_startDelayCounter > 0)
		return;
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
		/* Pounce when the player is above (line 231). */
		if (m_target->Y() < Y() - m_target->Height() && m_isTouchingGround)
			StartAction(ACTION_POUNCE);
		else
			StartAction(ACTION_WALK);
		break;
	case STATE_ENGAGE:
		if (!Chasing)
			StartAction(ACTION_WALK);
		break;
	case STATE_WANDER:
		if (Chasing)
			StartAction(ACTION_STOP);
		break;
	default:
		break;
	}
}

void EnemyObj_Wolf::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	/* EnemyObj_Wolf.cs line 316: the hit interrupts the current action
	 * (the C# stops the active LogicBlock and runs wolfHit). */
	if (m_action != ACTION_HIT)
		StartAction(ACTION_HIT);
	EnemyObj::HitEnemy(damage, collisionPt, isPlayer);
}

void EnemyObj_Wolf::ResetState()
{
	/* EnemyObj_Wolf.cs line 333 (ResetState). */
	m_startDelayCounter = m_startDelay;
	Chasing = false;
	m_action = ACTION_NONE;
	EnemyObj::ResetState();
}

} // namespace RogueCastle
