/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Zombie.cs. */
#include "EnemyObj_Zombie.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_Zombie::EnemyObj_Zombie(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyZombieLower_Character", target, difficulty)
{
	/* EnemyObj_Zombie.cs line 381 (constructor): asleep in the ground,
	 * on the last frame of the lower animation. */
	GoToFrame(TotalFrames());
	Lowered = true;
	Risen = false;
	ForceDraw(true);
	StopAnimation();
	Type = EnemyType::Zombie;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Zombie.cs line 14), so they collapse into one. */
void EnemyObj_Zombie::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Zombie_Miniboss_Name;
		MaxHealth = EnemyEV::Zombie_Miniboss_MaxHealth;
		Damage = EnemyEV::Zombie_Miniboss_Damage;
		XPValue = EnemyEV::Zombie_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Zombie_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Zombie_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Zombie_Miniboss_DropChance;
		Speed = EnemyEV::Zombie_Miniboss_Speed;
		TurnSpeed = EnemyEV::Zombie_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Zombie_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Zombie_Miniboss_Jump;
		CooldownTime = EnemyEV::Zombie_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Zombie_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Zombie_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Zombie_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Zombie_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Zombie_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Zombie_Miniboss_Scale.X, EnemyEV::Zombie_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Zombie_Miniboss_ProjectileScale.X, EnemyEV::Zombie_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Zombie_Miniboss_Tint.R, EnemyEV::Zombie_Miniboss_Tint.G, EnemyEV::Zombie_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Zombie_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Zombie_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Zombie_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Zombie_Miniboss_KnockBack.X, EnemyEV::Zombie_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Zombie_Expert_Name;
		MaxHealth = EnemyEV::Zombie_Expert_MaxHealth;
		Damage = EnemyEV::Zombie_Expert_Damage;
		XPValue = EnemyEV::Zombie_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Zombie_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Zombie_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Zombie_Expert_DropChance;
		Speed = EnemyEV::Zombie_Expert_Speed;
		TurnSpeed = EnemyEV::Zombie_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Zombie_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Zombie_Expert_Jump;
		CooldownTime = EnemyEV::Zombie_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Zombie_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Zombie_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Zombie_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Zombie_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Zombie_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Zombie_Expert_Scale.X, EnemyEV::Zombie_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Zombie_Expert_ProjectileScale.X, EnemyEV::Zombie_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Zombie_Expert_Tint.R, EnemyEV::Zombie_Expert_Tint.G, EnemyEV::Zombie_Expert_Tint.B));
		MeleeRadius = EnemyEV::Zombie_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Zombie_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Zombie_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Zombie_Expert_KnockBack.X, EnemyEV::Zombie_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Zombie_Advanced_Name;
		MaxHealth = EnemyEV::Zombie_Advanced_MaxHealth;
		Damage = EnemyEV::Zombie_Advanced_Damage;
		XPValue = EnemyEV::Zombie_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Zombie_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Zombie_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Zombie_Advanced_DropChance;
		Speed = EnemyEV::Zombie_Advanced_Speed;
		TurnSpeed = EnemyEV::Zombie_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Zombie_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Zombie_Advanced_Jump;
		CooldownTime = EnemyEV::Zombie_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Zombie_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Zombie_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Zombie_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Zombie_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Zombie_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Zombie_Advanced_Scale.X, EnemyEV::Zombie_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Zombie_Advanced_ProjectileScale.X, EnemyEV::Zombie_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Zombie_Advanced_Tint.R, EnemyEV::Zombie_Advanced_Tint.G, EnemyEV::Zombie_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Zombie_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Zombie_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Zombie_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Zombie_Advanced_KnockBack.X, EnemyEV::Zombie_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Zombie_Basic_Name;
		MaxHealth = EnemyEV::Zombie_Basic_MaxHealth;
		Damage = EnemyEV::Zombie_Basic_Damage;
		XPValue = EnemyEV::Zombie_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Zombie_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Zombie_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Zombie_Basic_DropChance;
		Speed = EnemyEV::Zombie_Basic_Speed;
		TurnSpeed = EnemyEV::Zombie_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Zombie_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Zombie_Basic_Jump;
		CooldownTime = EnemyEV::Zombie_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Zombie_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Zombie_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Zombie_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Zombie_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Zombie_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Zombie_Basic_Scale.X, EnemyEV::Zombie_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Zombie_Basic_ProjectileScale.X, EnemyEV::Zombie_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Zombie_Basic_Tint.R, EnemyEV::Zombie_Basic_Tint.G, EnemyEV::Zombie_Basic_Tint.B));
		MeleeRadius = EnemyEV::Zombie_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Zombie_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Zombie_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Zombie_Basic_KnockBack.X, EnemyEV::Zombie_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Zombie::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Zombie::RunBasicLogic()
{
	/* EnemyObj_Zombie.cs line 183: rise when engaged, walk once risen,
	 * sink when the player wanders away. */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		if (!Risen && !m_actionActive)
		{
			m_zombieState = ZOMBIE_RISING;
			m_actionActive = true;
			m_actionPhase = 0;
			CurrentSpeed(0);
			ChangeSprite("EnemyZombieRise_Character");
			PlayAnimation(false);
		}
		else if (Risen && !m_actionActive)
		{
			/* The walk cycle (walkTowardsLS): towards the player, 0.5 s
			 * per cycle (DelayLogicAction, line 152). */
			m_zombieState = ZOMBIE_WALKING;
			m_actionActive = true;
			if (SpriteName() != "EnemyZombieWalk_Character")
			{
				ChangeSprite("EnemyZombieWalk_Character");
				PlayAnimation(true);
			}
			HeadingX(m_target->X() >= X() ? 1 : -1);
			CurrentSpeed(Speed);
			m_actionTimer = 0.5f;
		}
		break;
	case STATE_WANDER:
		if (!Lowered && !m_actionActive)
		{
			m_zombieState = ZOMBIE_LOWERING;
			m_actionActive = true;
			m_actionPhase = 0;
			CurrentSpeed(0);
			ChangeSprite("EnemyZombieLower_Character");
			PlayAnimation(false);
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Zombie::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	switch (m_zombieState)
	{
	case ZOMBIE_RISING:
		/* riseLS (line 156): once the animation ends the zombie is up. */
		if (!IsAnimating())
		{
			Risen = true;
			Lowered = false;
			m_zombieState = ZOMBIE_WALKING;
			/* walkTowardsLS (line 148): the walk cycle, re-checked every
			 * 0.5 s (DelayLogicAction). */
			ChangeSprite("EnemyZombieWalk_Character");
			PlayAnimation(true);
			HeadingX(m_target->X() >= X() ? 1 : -1);
			CurrentSpeed(Speed);
			m_actionTimer = 0.5f;
		}
		break;
	case ZOMBIE_WALKING:
		if (m_actionTimer <= 0)
		{
			/* The cycle ends, so the state is re-evaluated next frame
			 * (the looping walk pauses between iterations in the C#). */
			m_actionActive = false;
			CurrentSpeed(0);
			m_zombieState = ZOMBIE_LOWERED;
		}
		break;
	case ZOMBIE_LOWERING:
		/* lowerLS (line 165). */
		if (!IsAnimating())
		{
			Risen = false;
			Lowered = true;
			m_actionActive = false;
			m_zombieState = ZOMBIE_LOWERED;
			StopAnimation();
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Zombie::ResetState()
{
	/* EnemyObj_Zombie.cs line 358 (ResetState). */
	Lowered = true;
	Risen = false;
	m_zombieState = ZOMBIE_LOWERED;
	m_actionActive = false;
	EnemyObj::ResetState();
	ChangeSprite("EnemyZombieLower_Character");
	GoToFrame(TotalFrames());
	StopAnimation();
}

} // namespace RogueCastle
