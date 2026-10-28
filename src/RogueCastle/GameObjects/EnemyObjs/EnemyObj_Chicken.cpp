/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Chicken.cs. */
#include "EnemyObj_Chicken.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Chicken::EnemyObj_Chicken(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyChickenRun_Character", target, difficulty)
{
	Type = EnemyType::Chicken;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Chicken.cs line 22), so they collapse into one. */
void EnemyObj_Chicken::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Chicken_Miniboss_Name;
		MaxHealth = EnemyEV::Chicken_Miniboss_MaxHealth;
		Damage = EnemyEV::Chicken_Miniboss_Damage;
		XPValue = EnemyEV::Chicken_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Chicken_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Chicken_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Chicken_Miniboss_DropChance;
		Speed = EnemyEV::Chicken_Miniboss_Speed;
		TurnSpeed = EnemyEV::Chicken_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Chicken_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Chicken_Miniboss_Jump;
		CooldownTime = EnemyEV::Chicken_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Chicken_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Chicken_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Chicken_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Chicken_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Chicken_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Chicken_Miniboss_Scale.X, EnemyEV::Chicken_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Chicken_Miniboss_ProjectileScale.X, EnemyEV::Chicken_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Chicken_Miniboss_Tint.R, EnemyEV::Chicken_Miniboss_Tint.G, EnemyEV::Chicken_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Chicken_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Chicken_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Chicken_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Chicken_Miniboss_KnockBack.X, EnemyEV::Chicken_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Chicken_Expert_Name;
		MaxHealth = EnemyEV::Chicken_Expert_MaxHealth;
		Damage = EnemyEV::Chicken_Expert_Damage;
		XPValue = EnemyEV::Chicken_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Chicken_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Chicken_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Chicken_Expert_DropChance;
		Speed = EnemyEV::Chicken_Expert_Speed;
		TurnSpeed = EnemyEV::Chicken_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Chicken_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Chicken_Expert_Jump;
		CooldownTime = EnemyEV::Chicken_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Chicken_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Chicken_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Chicken_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Chicken_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Chicken_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Chicken_Expert_Scale.X, EnemyEV::Chicken_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Chicken_Expert_ProjectileScale.X, EnemyEV::Chicken_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Chicken_Expert_Tint.R, EnemyEV::Chicken_Expert_Tint.G, EnemyEV::Chicken_Expert_Tint.B));
		MeleeRadius = EnemyEV::Chicken_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Chicken_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Chicken_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Chicken_Expert_KnockBack.X, EnemyEV::Chicken_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Chicken_Advanced_Name;
		MaxHealth = EnemyEV::Chicken_Advanced_MaxHealth;
		Damage = EnemyEV::Chicken_Advanced_Damage;
		XPValue = EnemyEV::Chicken_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Chicken_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Chicken_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Chicken_Advanced_DropChance;
		Speed = EnemyEV::Chicken_Advanced_Speed;
		TurnSpeed = EnemyEV::Chicken_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Chicken_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Chicken_Advanced_Jump;
		CooldownTime = EnemyEV::Chicken_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Chicken_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Chicken_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Chicken_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Chicken_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Chicken_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Chicken_Advanced_Scale.X, EnemyEV::Chicken_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Chicken_Advanced_ProjectileScale.X, EnemyEV::Chicken_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Chicken_Advanced_Tint.R, EnemyEV::Chicken_Advanced_Tint.G, EnemyEV::Chicken_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Chicken_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Chicken_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Chicken_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Chicken_Advanced_KnockBack.X, EnemyEV::Chicken_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Chicken_Basic_Name;
		MaxHealth = EnemyEV::Chicken_Basic_MaxHealth;
		Damage = EnemyEV::Chicken_Basic_Damage;
		XPValue = EnemyEV::Chicken_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Chicken_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Chicken_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Chicken_Basic_DropChance;
		Speed = EnemyEV::Chicken_Basic_Speed;
		TurnSpeed = EnemyEV::Chicken_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Chicken_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Chicken_Basic_Jump;
		CooldownTime = EnemyEV::Chicken_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Chicken_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Chicken_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Chicken_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Chicken_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Chicken_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Chicken_Basic_Scale.X, EnemyEV::Chicken_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Chicken_Basic_ProjectileScale.X, EnemyEV::Chicken_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Chicken_Basic_Tint.R, EnemyEV::Chicken_Basic_Tint.G, EnemyEV::Chicken_Basic_Tint.B));
		MeleeRadius = EnemyEV::Chicken_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Chicken_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Chicken_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Chicken_Basic_KnockBack.X, EnemyEV::Chicken_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Chicken::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Chicken::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Chicken::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;

	/* walkLeftLS/walkRightLS (lines 182, 187): run in the flipped
	 * direction for half a second to a second. */
	ChangeSprite("EnemyChickenRun_Character");
	PlayAnimation(true);
	if (action == ACTION_RUN_LEFT)
	{
		Flip(SpriteEffects::FlipHorizontally);
		Heading(Vector2(-1, 0));
	}
	else
	{
		Flip(SpriteEffects::None);
		Heading(Vector2(1, 0));
	}
	CurrentSpeed(Speed);
	m_actionTimer = RandRange(0.5f, 1.0f);
}

void EnemyObj_Chicken::RunBasicLogic()
{
	/* EnemyObj_Chicken.cs line 167: 50/50 left or right, whatever the
	 * state. */
	static const Action acts[] = { ACTION_RUN_LEFT, ACTION_RUN_RIGHT };
	static const int w[] = { 50, 50 };
	PickAction(w, 2, acts);
}

} // namespace RogueCastle
