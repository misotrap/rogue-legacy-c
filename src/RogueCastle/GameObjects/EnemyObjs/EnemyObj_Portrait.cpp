/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Portrait.cs. */
#include "EnemyObj_Portrait.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_Portrait::EnemyObj_Portrait(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyPortrait_Character", target, difficulty)
{
	Type = EnemyType::Portrait;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Portrait.cs line 22), so they collapse into one. */
void EnemyObj_Portrait::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Portrait_Miniboss_Name;
		MaxHealth = EnemyEV::Portrait_Miniboss_MaxHealth;
		Damage = EnemyEV::Portrait_Miniboss_Damage;
		XPValue = EnemyEV::Portrait_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Miniboss_DropChance;
		Speed = EnemyEV::Portrait_Miniboss_Speed;
		TurnSpeed = EnemyEV::Portrait_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Miniboss_Jump;
		CooldownTime = EnemyEV::Portrait_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Miniboss_Scale.X, EnemyEV::Portrait_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Miniboss_ProjectileScale.X, EnemyEV::Portrait_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Miniboss_Tint.R, EnemyEV::Portrait_Miniboss_Tint.G, EnemyEV::Portrait_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Miniboss_KnockBack.X, EnemyEV::Portrait_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Portrait_Expert_Name;
		MaxHealth = EnemyEV::Portrait_Expert_MaxHealth;
		Damage = EnemyEV::Portrait_Expert_Damage;
		XPValue = EnemyEV::Portrait_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Expert_DropChance;
		Speed = EnemyEV::Portrait_Expert_Speed;
		TurnSpeed = EnemyEV::Portrait_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Expert_Jump;
		CooldownTime = EnemyEV::Portrait_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Expert_Scale.X, EnemyEV::Portrait_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Expert_ProjectileScale.X, EnemyEV::Portrait_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Expert_Tint.R, EnemyEV::Portrait_Expert_Tint.G, EnemyEV::Portrait_Expert_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Expert_KnockBack.X, EnemyEV::Portrait_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Portrait_Advanced_Name;
		MaxHealth = EnemyEV::Portrait_Advanced_MaxHealth;
		Damage = EnemyEV::Portrait_Advanced_Damage;
		XPValue = EnemyEV::Portrait_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Advanced_DropChance;
		Speed = EnemyEV::Portrait_Advanced_Speed;
		TurnSpeed = EnemyEV::Portrait_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Advanced_Jump;
		CooldownTime = EnemyEV::Portrait_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Advanced_Scale.X, EnemyEV::Portrait_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Advanced_ProjectileScale.X, EnemyEV::Portrait_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Advanced_Tint.R, EnemyEV::Portrait_Advanced_Tint.G, EnemyEV::Portrait_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Advanced_KnockBack.X, EnemyEV::Portrait_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Portrait_Basic_Name;
		MaxHealth = EnemyEV::Portrait_Basic_MaxHealth;
		Damage = EnemyEV::Portrait_Basic_Damage;
		XPValue = EnemyEV::Portrait_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Portrait_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Portrait_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Portrait_Basic_DropChance;
		Speed = EnemyEV::Portrait_Basic_Speed;
		TurnSpeed = EnemyEV::Portrait_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Portrait_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Portrait_Basic_Jump;
		CooldownTime = EnemyEV::Portrait_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Portrait_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Portrait_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Portrait_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Portrait_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Portrait_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Portrait_Basic_Scale.X, EnemyEV::Portrait_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Portrait_Basic_ProjectileScale.X, EnemyEV::Portrait_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Portrait_Basic_Tint.R, EnemyEV::Portrait_Basic_Tint.G, EnemyEV::Portrait_Basic_Tint.B));
		MeleeRadius = EnemyEV::Portrait_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Portrait_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Portrait_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Portrait_Basic_KnockBack.X, EnemyEV::Portrait_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Portrait::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Portrait::FireSpread()
{
	/* The ghost projectile spreads: ThrowProjectiles fires the
	 * diagonals (135/45/-45/-135), the advanced and expert sets the
	 * cross (0/90/-90/180). */
	static const float diagonals[4] = { 135, 45, -45, -135 };
	static const float cross[4] = { 0, 90, -90, 180 };
	const float *angles = DifficultyType == MINIBOSS ? diagonals : cross;
	for (int i = 0; i < 4; i++)
	{
		float rad = angles[i] * PI / 180.0f;
		m_levelScreen->FireProjectile("GhostProjectile_Sprite", Position(),
		                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
		                              (int) ProjectileDamage, ProjectileScale, false, this);
	}
}

void EnemyObj_Portrait::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 1;

	switch (action)
	{
	case ACTION_SHAKE:
		/* basicWarningLS (line 186): shake for a second, rest a second
		 * (the visual offset is not ported). */
		CurrentSpeed(0);
		m_shakeOn = true;
		m_actionTimer = 1.0f;
		break;
	case ACTION_CHASE:
		/* moveTowardsLS (line 194): fly at the player; the higher
		 * difficulties open with the spread (ThrowXProjectiles). */
		ChangeSprite("EnemyPortrait_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed);
		if (DifficultyType != BASIC)
			FireSpread();
		m_actionTimer = DifficultyType == MINIBOSS ? 1.25f
		              : (DifficultyType == ADVANCED || DifficultyType == EXPERT ? 1.75f : 1.0f);
		break;
	default:
		break;
	}
}

void EnemyObj_Portrait::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_SHAKE:
		if (m_actionTimer <= 0)
		{
			if (m_shakeOn)
			{
				m_shakeOn = false;
				m_actionTimer = 1.0f;
			}
			else
				m_actionPhase = 1;
		}
		break;
	case ACTION_CHASE:
		/* Re-aim at the player (ChaseLogicAction). */
		{
			float dx = m_target->X() - X();
			float dy = m_target->Y() - Y();
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 1)
				Heading(Vector2(dx / len, dy / len));
		}
		if (m_actionTimer <= 0)
		{
			CurrentSpeed(0);
			m_actionPhase = 1;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Portrait::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Portrait::RunBasicLogic()
{
	/* EnemyObj_Portrait.cs line 283: shake when the player is engaged,
	 * fly after them when they wander away. */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		if (!m_actionActive || m_action == ACTION_CHASE)
			StartAction(ACTION_SHAKE);
		break;
	default: /* WANDER */
		if (!m_actionActive || m_action == ACTION_SHAKE)
			StartAction(ACTION_CHASE);
		break;
	}
}

} // namespace RogueCastle
