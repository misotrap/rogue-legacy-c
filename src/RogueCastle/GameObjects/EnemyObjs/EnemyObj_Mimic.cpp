/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Mimic.cs. */
#include "EnemyObj_Mimic.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_Mimic::EnemyObj_Mimic(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyMimicIdle_Character", target, difficulty)
{
	/* EnemyObj_Mimic.cs line 310 (constructor). */
	Type = EnemyType::Mimic;
	LockFlip = true;   /* line 183: it faces where it was placed until provoked */
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Mimic.cs line 28), so they collapse into one. */
void EnemyObj_Mimic::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Mimic_Miniboss_Name;
		MaxHealth = EnemyEV::Mimic_Miniboss_MaxHealth;
		Damage = EnemyEV::Mimic_Miniboss_Damage;
		XPValue = EnemyEV::Mimic_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Mimic_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Mimic_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Mimic_Miniboss_DropChance;
		Speed = EnemyEV::Mimic_Miniboss_Speed;
		TurnSpeed = EnemyEV::Mimic_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Mimic_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Mimic_Miniboss_Jump;
		CooldownTime = EnemyEV::Mimic_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Mimic_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Mimic_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Mimic_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Mimic_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Mimic_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Mimic_Miniboss_Scale.X, EnemyEV::Mimic_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Mimic_Miniboss_ProjectileScale.X, EnemyEV::Mimic_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Mimic_Miniboss_Tint.R, EnemyEV::Mimic_Miniboss_Tint.G, EnemyEV::Mimic_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Mimic_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Mimic_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Mimic_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Mimic_Miniboss_KnockBack.X, EnemyEV::Mimic_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Mimic_Expert_Name;
		MaxHealth = EnemyEV::Mimic_Expert_MaxHealth;
		Damage = EnemyEV::Mimic_Expert_Damage;
		XPValue = EnemyEV::Mimic_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Mimic_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Mimic_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Mimic_Expert_DropChance;
		Speed = EnemyEV::Mimic_Expert_Speed;
		TurnSpeed = EnemyEV::Mimic_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Mimic_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Mimic_Expert_Jump;
		CooldownTime = EnemyEV::Mimic_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Mimic_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Mimic_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Mimic_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Mimic_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Mimic_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Mimic_Expert_Scale.X, EnemyEV::Mimic_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Mimic_Expert_ProjectileScale.X, EnemyEV::Mimic_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Mimic_Expert_Tint.R, EnemyEV::Mimic_Expert_Tint.G, EnemyEV::Mimic_Expert_Tint.B));
		MeleeRadius = EnemyEV::Mimic_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Mimic_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Mimic_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Mimic_Expert_KnockBack.X, EnemyEV::Mimic_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Mimic_Advanced_Name;
		MaxHealth = EnemyEV::Mimic_Advanced_MaxHealth;
		Damage = EnemyEV::Mimic_Advanced_Damage;
		XPValue = EnemyEV::Mimic_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Mimic_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Mimic_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Mimic_Advanced_DropChance;
		Speed = EnemyEV::Mimic_Advanced_Speed;
		TurnSpeed = EnemyEV::Mimic_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Mimic_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Mimic_Advanced_Jump;
		CooldownTime = EnemyEV::Mimic_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Mimic_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Mimic_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Mimic_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Mimic_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Mimic_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Mimic_Advanced_Scale.X, EnemyEV::Mimic_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Mimic_Advanced_ProjectileScale.X, EnemyEV::Mimic_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Mimic_Advanced_Tint.R, EnemyEV::Mimic_Advanced_Tint.G, EnemyEV::Mimic_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Mimic_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Mimic_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Mimic_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Mimic_Advanced_KnockBack.X, EnemyEV::Mimic_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Mimic_Basic_Name;
		MaxHealth = EnemyEV::Mimic_Basic_MaxHealth;
		Damage = EnemyEV::Mimic_Basic_Damage;
		XPValue = EnemyEV::Mimic_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Mimic_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Mimic_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Mimic_Basic_DropChance;
		Speed = EnemyEV::Mimic_Basic_Speed;
		TurnSpeed = EnemyEV::Mimic_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Mimic_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Mimic_Basic_Jump;
		CooldownTime = EnemyEV::Mimic_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Mimic_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Mimic_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Mimic_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Mimic_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Mimic_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Mimic_Basic_Scale.X, EnemyEV::Mimic_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Mimic_Basic_ProjectileScale.X, EnemyEV::Mimic_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Mimic_Basic_Tint.R, EnemyEV::Mimic_Basic_Tint.G, EnemyEV::Mimic_Basic_Tint.B));
		MeleeRadius = EnemyEV::Mimic_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Mimic_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Mimic_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Mimic_Basic_KnockBack.X, EnemyEV::Mimic_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Mimic::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Mimic::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_SHAKE:
		/* basicWarningLS (line 188): shake, then idle for 3 s. */
		ChangeSprite("EnemyMimicShake_Character");
		PlayAnimation(false);
		m_endPhase = 1;
		break;
	case ACTION_JUMP:
		/* jumpTowardsLS (line 194): face the player, hop at them. */
		if (!m_isTouchingGround)
			return;
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("EnemyMimicAttack_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed);
		AccelerationY(-JumpHeight);
		m_actionTimer = 0.3f;   /* the air time before the ground check */
		m_endPhase = 1;
		break;
	default:
		break;
	}
}

void EnemyObj_Mimic::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_SHAKE:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			ChangeSprite("EnemyMimicIdle_Character");
			PlayAnimation(false);
			m_actionPhase = 1;
			m_actionTimer = 3.0f;
		}
		break;
	case ACTION_JUMP:
		/* GroundCheckLogicAction: the hop ends on the landing. */
		if (m_actionPhase == 0 && m_actionTimer <= 0 && m_isTouchingGround)
			m_actionPhase = 1;
		break;
	default:
		break;
	}
}

void EnemyObj_Mimic::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* CollisionResponse (line 296): touching the hero wakes it */
	if (!m_isAttacking && !IsKilled && m_target != nullptr && Bounds().Intersects(m_target->TerrainBounds()))
		Provoke();
	UpdateAction();
}

void EnemyObj_Mimic::Provoke()
{
	m_isAttacking = true;
	m_actionActive = false;   /* StopLogicBlock */
	LockFlip = false;
}

void EnemyObj_Mimic::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	if (!m_isAttacking) Provoke();   /* line 283 */
	EnemyObj::HitEnemy(damage, collisionPt, isPlayer);
}

void EnemyObj_Mimic::RunBasicLogic()
{
	/* EnemyObj_Mimic.cs line 177: shake as a warning when the player
	 * lingers, hop at them when they are close. */
	/* line 220: it only shakes until hit or touched, then hops at the
	 * hero whatever the distance */
	if (!m_actionActive)
		StartAction(m_isAttacking ? ACTION_JUMP : ACTION_SHAKE);
}

} // namespace RogueCastle
