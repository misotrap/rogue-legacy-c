/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Starburst.cs. */
#include "EnemyObj_Starburst.h"
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

EnemyObj_Starburst::EnemyObj_Starburst(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyStarburstIdle_Character", target, difficulty)
{
	Type = EnemyType::Starburst;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Starburst.cs line 22), so they collapse into one. */
void EnemyObj_Starburst::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Starburst_Miniboss_Name;
		MaxHealth = EnemyEV::Starburst_Miniboss_MaxHealth;
		Damage = EnemyEV::Starburst_Miniboss_Damage;
		XPValue = EnemyEV::Starburst_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Starburst_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Starburst_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Starburst_Miniboss_DropChance;
		Speed = EnemyEV::Starburst_Miniboss_Speed;
		TurnSpeed = EnemyEV::Starburst_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Starburst_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Starburst_Miniboss_Jump;
		CooldownTime = EnemyEV::Starburst_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Starburst_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Starburst_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Starburst_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Starburst_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Starburst_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Starburst_Miniboss_Scale.X, EnemyEV::Starburst_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Starburst_Miniboss_ProjectileScale.X, EnemyEV::Starburst_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Starburst_Miniboss_Tint.R, EnemyEV::Starburst_Miniboss_Tint.G, EnemyEV::Starburst_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Starburst_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Starburst_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Starburst_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Starburst_Miniboss_KnockBack.X, EnemyEV::Starburst_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Starburst_Expert_Name;
		MaxHealth = EnemyEV::Starburst_Expert_MaxHealth;
		Damage = EnemyEV::Starburst_Expert_Damage;
		XPValue = EnemyEV::Starburst_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Starburst_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Starburst_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Starburst_Expert_DropChance;
		Speed = EnemyEV::Starburst_Expert_Speed;
		TurnSpeed = EnemyEV::Starburst_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Starburst_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Starburst_Expert_Jump;
		CooldownTime = EnemyEV::Starburst_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Starburst_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Starburst_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Starburst_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Starburst_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Starburst_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Starburst_Expert_Scale.X, EnemyEV::Starburst_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Starburst_Expert_ProjectileScale.X, EnemyEV::Starburst_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Starburst_Expert_Tint.R, EnemyEV::Starburst_Expert_Tint.G, EnemyEV::Starburst_Expert_Tint.B));
		MeleeRadius = EnemyEV::Starburst_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Starburst_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Starburst_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Starburst_Expert_KnockBack.X, EnemyEV::Starburst_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Starburst_Advanced_Name;
		MaxHealth = EnemyEV::Starburst_Advanced_MaxHealth;
		Damage = EnemyEV::Starburst_Advanced_Damage;
		XPValue = EnemyEV::Starburst_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Starburst_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Starburst_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Starburst_Advanced_DropChance;
		Speed = EnemyEV::Starburst_Advanced_Speed;
		TurnSpeed = EnemyEV::Starburst_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Starburst_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Starburst_Advanced_Jump;
		CooldownTime = EnemyEV::Starburst_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Starburst_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Starburst_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Starburst_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Starburst_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Starburst_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Starburst_Advanced_Scale.X, EnemyEV::Starburst_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Starburst_Advanced_ProjectileScale.X, EnemyEV::Starburst_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Starburst_Advanced_Tint.R, EnemyEV::Starburst_Advanced_Tint.G, EnemyEV::Starburst_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Starburst_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Starburst_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Starburst_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Starburst_Advanced_KnockBack.X, EnemyEV::Starburst_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Starburst_Basic_Name;
		MaxHealth = EnemyEV::Starburst_Basic_MaxHealth;
		Damage = EnemyEV::Starburst_Basic_Damage;
		XPValue = EnemyEV::Starburst_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Starburst_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Starburst_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Starburst_Basic_DropChance;
		Speed = EnemyEV::Starburst_Basic_Speed;
		TurnSpeed = EnemyEV::Starburst_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Starburst_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Starburst_Basic_Jump;
		CooldownTime = EnemyEV::Starburst_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Starburst_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Starburst_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Starburst_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Starburst_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Starburst_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Starburst_Basic_Scale.X, EnemyEV::Starburst_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Starburst_Basic_ProjectileScale.X, EnemyEV::Starburst_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Starburst_Basic_Tint.R, EnemyEV::Starburst_Basic_Tint.G, EnemyEV::Starburst_Basic_Tint.B));
		MeleeRadius = EnemyEV::Starburst_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Starburst_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Starburst_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Starburst_Basic_KnockBack.X, EnemyEV::Starburst_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Starburst::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Starburst::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_FIRE:
		/* fireProjectileBasicLS (line 196): a pause, the volley, then
		 * back to idle. */
		ChangeSprite("EnemyStarburstAttack_Character");
		PlayAnimation(true);
		m_actionTimer = FireballDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	case ACTION_NOTHING:
		/* doNothing (line 294). */
		m_actionTimer = 0.2f + 0.3f * ((float) rand() / (float) RAND_MAX);
		break;
	default:
		break;
	}
}

void EnemyObj_Starburst::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_FIRE:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The volley: the cross (0/-90/90/180), the advanced's
			 * rotated cross (45/135/-45/-135), the expert's eight. */
			static const float cross[4] = { 0, -90, 90, 180 };
			static const float rotated[4] = { 45, 135, -45, -135 };
			static const float eight[8] = { 45, -45, 135, -135, -90, 90, 180, 0 };
			const float *angles = cross;
			int count = 4;
			if (DifficultyType == EXPERT)
			{
				angles = eight;
				count = 8;
			}
			else if (DifficultyType == ADVANCED || DifficultyType == MINIBOSS)
				angles = rotated;
			Audio::PlaySound("Eyeball_ProjectileAttack");   /* EnemyObj_Starburst.cs line 162 */
			for (int i = 0; i < count; i++)
			{
				float rad = angles[i] * PI / 180.0f;
				m_levelScreen->FireProjectile("TurretProjectile_Sprite", Position(),
				                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
				                              (int) ProjectileDamage, ProjectileScale,
				                              false, this);
			}
			ChangeSprite("EnemyStarburstIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 1;
			m_actionTimer = 1.0f;   /* DelayLogicAction(1.0f, 1.0f) */
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
			m_actionPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Starburst::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Starburst::RunBasicLogic()
{
	/* EnemyObj_Starburst.cs line 322: fire when engaged, rest when
	 * wandering. */
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
		if (!m_actionActive)
			StartAction(ACTION_FIRE);
		break;
	default: /* WANDER */
		if (!m_actionActive)
			StartAction(ACTION_NOTHING);
		break;
	}
}

} // namespace RogueCastle
