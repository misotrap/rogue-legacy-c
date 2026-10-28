/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_ShieldKnight.cs. */
#include "EnemyObj_ShieldKnight.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include <cstdlib>
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_ShieldKnight::EnemyObj_ShieldKnight(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyShieldKnightIdle_Character", target, difficulty)
{
	Type = EnemyType::ShieldKnight;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_ShieldKnight.cs line 20), so they collapse into one. */
void EnemyObj_ShieldKnight::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::ShieldKnight_Miniboss_Name;
		MaxHealth = EnemyEV::ShieldKnight_Miniboss_MaxHealth;
		Damage = EnemyEV::ShieldKnight_Miniboss_Damage;
		XPValue = EnemyEV::ShieldKnight_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::ShieldKnight_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::ShieldKnight_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::ShieldKnight_Miniboss_DropChance;
		Speed = EnemyEV::ShieldKnight_Miniboss_Speed;
		TurnSpeed = EnemyEV::ShieldKnight_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::ShieldKnight_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::ShieldKnight_Miniboss_Jump;
		CooldownTime = EnemyEV::ShieldKnight_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::ShieldKnight_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::ShieldKnight_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::ShieldKnight_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::ShieldKnight_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::ShieldKnight_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::ShieldKnight_Miniboss_Scale.X, EnemyEV::ShieldKnight_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::ShieldKnight_Miniboss_ProjectileScale.X, EnemyEV::ShieldKnight_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::ShieldKnight_Miniboss_Tint.R, EnemyEV::ShieldKnight_Miniboss_Tint.G, EnemyEV::ShieldKnight_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::ShieldKnight_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::ShieldKnight_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::ShieldKnight_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::ShieldKnight_Miniboss_KnockBack.X, EnemyEV::ShieldKnight_Miniboss_KnockBack.Y);
		ShieldKnockback = Vector2(1050, 1150);   /* line 143 */
		break;
	case EXPERT:
		Name = EnemyEV::ShieldKnight_Expert_Name;
		MaxHealth = EnemyEV::ShieldKnight_Expert_MaxHealth;
		Damage = EnemyEV::ShieldKnight_Expert_Damage;
		XPValue = EnemyEV::ShieldKnight_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::ShieldKnight_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::ShieldKnight_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::ShieldKnight_Expert_DropChance;
		Speed = EnemyEV::ShieldKnight_Expert_Speed;
		TurnSpeed = EnemyEV::ShieldKnight_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::ShieldKnight_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::ShieldKnight_Expert_Jump;
		CooldownTime = EnemyEV::ShieldKnight_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::ShieldKnight_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::ShieldKnight_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::ShieldKnight_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::ShieldKnight_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::ShieldKnight_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::ShieldKnight_Expert_Scale.X, EnemyEV::ShieldKnight_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::ShieldKnight_Expert_ProjectileScale.X, EnemyEV::ShieldKnight_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::ShieldKnight_Expert_Tint.R, EnemyEV::ShieldKnight_Expert_Tint.G, EnemyEV::ShieldKnight_Expert_Tint.B));
		MeleeRadius = EnemyEV::ShieldKnight_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::ShieldKnight_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::ShieldKnight_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::ShieldKnight_Expert_KnockBack.X, EnemyEV::ShieldKnight_Expert_KnockBack.Y);
		ShieldKnockback = Vector2(1550, 1650);   /* line 103 */
		break;
	case ADVANCED:
		Name = EnemyEV::ShieldKnight_Advanced_Name;
		MaxHealth = EnemyEV::ShieldKnight_Advanced_MaxHealth;
		Damage = EnemyEV::ShieldKnight_Advanced_Damage;
		XPValue = EnemyEV::ShieldKnight_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::ShieldKnight_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::ShieldKnight_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::ShieldKnight_Advanced_DropChance;
		Speed = EnemyEV::ShieldKnight_Advanced_Speed;
		TurnSpeed = EnemyEV::ShieldKnight_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::ShieldKnight_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::ShieldKnight_Advanced_Jump;
		CooldownTime = EnemyEV::ShieldKnight_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::ShieldKnight_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::ShieldKnight_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::ShieldKnight_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::ShieldKnight_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::ShieldKnight_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::ShieldKnight_Advanced_Scale.X, EnemyEV::ShieldKnight_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::ShieldKnight_Advanced_ProjectileScale.X, EnemyEV::ShieldKnight_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::ShieldKnight_Advanced_Tint.R, EnemyEV::ShieldKnight_Advanced_Tint.G, EnemyEV::ShieldKnight_Advanced_Tint.B));
		MeleeRadius = EnemyEV::ShieldKnight_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::ShieldKnight_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::ShieldKnight_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::ShieldKnight_Advanced_KnockBack.X, EnemyEV::ShieldKnight_Advanced_KnockBack.Y);
		ShieldKnockback = Vector2(1200, 1350);   /* line 63 */
		break;
	default: /* BASIC */
		Name = EnemyEV::ShieldKnight_Basic_Name;
		MaxHealth = EnemyEV::ShieldKnight_Basic_MaxHealth;
		Damage = EnemyEV::ShieldKnight_Basic_Damage;
		XPValue = EnemyEV::ShieldKnight_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::ShieldKnight_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::ShieldKnight_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::ShieldKnight_Basic_DropChance;
		Speed = EnemyEV::ShieldKnight_Basic_Speed;
		TurnSpeed = EnemyEV::ShieldKnight_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::ShieldKnight_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::ShieldKnight_Basic_Jump;
		CooldownTime = EnemyEV::ShieldKnight_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::ShieldKnight_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::ShieldKnight_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::ShieldKnight_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::ShieldKnight_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::ShieldKnight_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::ShieldKnight_Basic_Scale.X, EnemyEV::ShieldKnight_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::ShieldKnight_Basic_ProjectileScale.X, EnemyEV::ShieldKnight_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::ShieldKnight_Basic_Tint.R, EnemyEV::ShieldKnight_Basic_Tint.G, EnemyEV::ShieldKnight_Basic_Tint.B));
		MeleeRadius = EnemyEV::ShieldKnight_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::ShieldKnight_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::ShieldKnight_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::ShieldKnight_Basic_KnockBack.X, EnemyEV::ShieldKnight_Basic_KnockBack.Y);
		break;
	}
	/* The shield blocks even more on higher difficulties (line 17 area). */
	if (DifficultyType == EXPERT || DifficultyType == MINIBOSS)
		m_blockDmgReduction = 0.7f;
}

void EnemyObj_ShieldKnight::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_ShieldKnight::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_STOP:
		/* walkStopLS (line 147): facing locked, just stop. */
		ChangeSprite("EnemyShieldKnightIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.5f, 2.0f);
		break;
	case ACTION_WALK:
		/* walkTowardsLS (line 153): walk along the locked facing. */
		ChangeSprite("EnemyShieldKnightWalk_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.5f, 2.0f);
		break;
	case ACTION_TURN:
		/* turnLS (line 159): the turn animation, flip, walk on. */
		CurrentSpeed(0);
		Audio::PlaySound("ShieldKnight_Turn");   /* EnemyObj_ShieldKnight.cs line 166 */
		ChangeSprite("EnemyShieldKnightTurnIn_Character");
		PlayAnimation(1, 2, false);
		m_endPhase = 3;
		break;
	default:
		break;
	}
}

void EnemyObj_ShieldKnight::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_TURN:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			PlayAnimation(3, TotalFrames(), false);
			HeadingX(-HeadingX());   /* the knight now faces the player */
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ChangeSprite("EnemyShieldKnightTurnOut_Character");
			PlayAnimation(true);
			CurrentSpeed(Speed);
			m_actionPhase = 2;
		}
		else if (m_actionPhase == 2)
			m_actionPhase = 3;   /* walks on; the next walk cycle re-checks */
		break;
	default:
		break;
	}
}

void EnemyObj_ShieldKnight::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_ShieldKnight::RunBasicLogic()
{
	/* EnemyObj_ShieldKnight.cs line 219: turn around when the player is
	 * behind the locked facing, walk when engaged, stop otherwise. */
	bool playerBehind = (m_target->X() > X() && HeadingX() < 0)
	                 || (m_target->X() < X() && HeadingX() >= 0);
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		if (playerBehind)
			StartAction(ACTION_TURN);
		else
			StartAction(ACTION_WALK);
		break;
	case STATE_WANDER:
		if (playerBehind)
			StartAction(ACTION_TURN);
		else
			StartAction(ACTION_STOP);
		break;
	default:
		break;
	}
}

void EnemyObj_ShieldKnight::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	/* EnemyObj_ShieldKnight.cs line 409: frontal sword hits are blocked
	 * (reduced damage, the player bounces off the shield). */
	if (isPlayer && !m_target->IsAirAttacking)
	{
		bool frontal = (Flip() == SpriteEffects::None && m_target->X() > X())
		            || (Flip() == SpriteEffects::FlipHorizontally && m_target->X() < X());
		if (frontal)
		{
			damage = (int) (damage * (1 - m_blockDmgReduction));
			/* ShieldKnockback (line 371): push the player away. */
			if (m_target->Bounds().Left() + m_target->Bounds().Width / 2 < X())
				m_target->AccelerationX(-ShieldKnockback.X);
			else
				m_target->AccelerationX(ShieldKnockback.X);
			m_target->AccelerationY(-ShieldKnockback.Y);
		}
	}
	EnemyObj::HitEnemy(damage, collisionPt, isPlayer);
}

void EnemyObj_ShieldKnight::ResetState()
{
	/* Keep the heading: the base resets it, the knight's facing is
	 * whatever it was walking in. */
	float headingX = HeadingX();
	EnemyObj::ResetState();
	HeadingX(headingX == 0 ? 1 : headingX);
	m_action = ACTION_NONE;
}

} // namespace RogueCastle
