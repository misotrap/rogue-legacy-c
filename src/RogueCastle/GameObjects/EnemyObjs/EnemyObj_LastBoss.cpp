/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_LastBoss.cs (subset). */
#include "EnemyObj_LastBoss.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_LastBoss::EnemyObj_LastBoss(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("PlayerIdle_Character", target, difficulty)
{
	/* EnemyObj_LastBoss.cs line 1779: Johannes is the player, grey. */
	Type = EnemyType::LastBoss;
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->TextureColor(Color(100, 100, 100));
}

void EnemyObj_LastBoss::InitializeEV()
{
	/* The first form uses the basic stats; the second form the advanced
	 * ones (EnemyObj_LastBoss.cs line 60). */
	MaxHealth = EnemyEV::LastBoss_Basic_MaxHealth;
	Damage = EnemyEV::LastBoss_Basic_Damage;
	XPValue = EnemyEV::LastBoss_Basic_XPValue;
	Speed = EnemyEV::LastBoss_Basic_Speed;
	JumpHeight = EnemyEV::LastBoss_Basic_Jump;
	CooldownTime = EnemyEV::LastBoss_Basic_Cooldown;
	TurnSpeed = EnemyEV::LastBoss_Basic_TurnSpeed;
	ProjectileSpeed = EnemyEV::LastBoss_Basic_ProjectileSpeed;
	AnimationDelay(1 / EnemyEV::LastBoss_Basic_AnimationDelay);
	AlwaysFaceTarget = EnemyEV::LastBoss_Basic_AlwaysFaceTarget;
	CanFallOffLedges = EnemyEV::LastBoss_Basic_CanFallOffLedges;
	CanBeKnockedBack = EnemyEV::LastBoss_Basic_CanBeKnockedBack;
	IsWeightedFlag = EnemyEV::LastBoss_Basic_IsWeighted;
	Scale(Vector2(EnemyEV::LastBoss_Basic_Scale.X, EnemyEV::LastBoss_Basic_Scale.Y));
	ProjectileScale = Vector2(EnemyEV::LastBoss_Basic_ProjectileScale.X, EnemyEV::LastBoss_Basic_ProjectileScale.Y);
	MeleeRadius = EnemyEV::LastBoss_Basic_MeleeRadius;
	ProjectileRadius = EnemyEV::LastBoss_Basic_ProjectileRadius;
	EngageRadius = EnemyEV::LastBoss_Basic_EngageRadius;
	ProjectileDamage = Damage;
	KnockBack = Vector2(EnemyEV::LastBoss_Basic_KnockBack.X, EnemyEV::LastBoss_Basic_KnockBack.Y);
	Name = EnemyEV::LastBoss_Basic_Name;
}

void EnemyObj_LastBoss::SecondFormTransition()
{
	/* MovePlayerTo (line 1420): the Fountain takes over with the
	 * advanced stats. The cinematics (screen borders, the player
	 * repositioning) are not ported. */
	m_inSecondForm = true;
	m_formKilled = false;
	m_actionActive = false;
	m_action = ACTION_NONE;
	Name = EnemyEV::LastBoss_Advanced_Name;
	MaxHealth = EnemyEV::LastBoss_Advanced_MaxHealth;
	Damage = EnemyEV::LastBoss_Advanced_Damage;
	Speed = EnemyEV::LastBoss_Advanced_Speed;
	JumpHeight = EnemyEV::LastBoss_Advanced_Jump;
	CooldownTime = EnemyEV::LastBoss_Advanced_Cooldown;
	TurnSpeed = EnemyEV::LastBoss_Advanced_TurnSpeed;
	ProjectileSpeed = EnemyEV::LastBoss_Advanced_ProjectileSpeed;
	MeleeRadius = EnemyEV::LastBoss_Advanced_MeleeRadius;
	ProjectileRadius = EnemyEV::LastBoss_Advanced_ProjectileRadius;
	EngageRadius = EnemyEV::LastBoss_Advanced_EngageRadius;
	ProjectileDamage = Damage;
	KnockBack = Vector2(EnemyEV::LastBoss_Advanced_KnockBack.X, EnemyEV::LastBoss_Advanced_KnockBack.Y);
	CurrentHealth = MaxHealth;
	ChangeSprite("EnemyLastBossIdle_Character");
	PlayAnimation(true);
	/* The Fountain is not the grey player mirror. */
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->TextureColor(Color(255, 255, 255));
}

void EnemyObj_LastBoss::Kill(bool giveXP)
{
	/* The first form does not die: it becomes the Fountain (line 1420);
	 * the second form's death is the real one. */
	if (!m_inSecondForm)
	{
		m_formKilled = true;
		SecondFormTransition();
		return;
	}
	EnemyObj::Kill(giveXP);
}

void EnemyObj_LastBoss::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_LastBoss::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	if (m_inSecondForm)
	{
		switch (action)
		{
		case ACTION_SF_WALK:
			/* walkTowardsSF (line 338). */
			ChangeSprite("EnemyLastBossRun_Character");
			PlayAnimation(true);
			HeadingX(m_target->X() >= X() ? 1 : -1);
			CurrentSpeed(Speed);
			m_actionTimer = RandRange(0.3f, 0.75f);
			break;
		case ACTION_SF_ATTACK:
			/* attackSF (line 357): the melee swing. */
			CurrentSpeed(0);
			HeadingX(m_target->X() >= X() ? 1 : -1);
			ChangeSprite("EnemyLastBossAttack_Character");
			PlayAnimation(true);
			m_runCooldown = true;
			m_cooldownTimer = CooldownTime;
			m_endPhase = 1;
			break;
		case ACTION_SF_CAST_SWORDS:
			/* castSwordsLeftSF/castSwordRightSF (line 368+): swords fly
			 * across the room at the player's height. */
			CurrentSpeed(0);
			ChangeSprite("EnemyLastBossSpell_Character");
			PlayAnimation(true);
			m_runCooldown = true;
			m_cooldownTimer = CooldownTime;
			m_endPhase = 1;
			break;
		case ACTION_SF_CAST_SPEARS:
			/* castSpearsSF (line 368): spears rise around the player. */
			CurrentSpeed(0);
			ChangeSprite("EnemyLastBossSpell2_Character");
			PlayAnimation(true);
			m_runCooldown = true;
			m_cooldownTimer = CooldownTime;
			m_endPhase = 1;
			break;
		default:
			break;
		}
		return;
	}

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS/walkAwayLS (lines 193, 202). */
		ChangeSprite("PlayerWalking_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                               : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.3f, 0.75f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 211). */
		ChangeSprite("PlayerIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.25f, 0.5f);
		break;
	case ACTION_JUMP:
		/* jumpLS (line 288): leap at the player with a sword swing. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("PlayerAttacking3_Character");
		PlayAnimation(true);
		CurrentSpeed(Speed * 1.5f);
		AccelerationY(-JumpHeight);
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 1;
		break;
	case ACTION_MOVE_ATTACK:
		/* moveAttackLS (line 232): the swing while closing in. */
		HeadingX(m_target->X() >= X() ? 1 : -1);
		CurrentSpeed(Speed);
		ChangeSprite("PlayerAttacking3_Character");
		PlayAnimation(true);
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 1;
		break;
	case ACTION_THROW_DAGGER:
		/* throwDaggerLS (line 256): the dagger at the player. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		AnimationDelay(1 / 30.0f);
		ChangeSprite("PlayerLevelUp_Character");
		PlayAnimation(true);
		m_actionTimer = 0.25f;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	case ACTION_DASH:
		/* dashLS (line 318): the quick dash at the player. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("PlayerWalking_Character");
		PlayAnimation(true);
		m_actionTimer = m_lastBossAttackDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_LastBoss::UpdateAction()
{
	if (m_inSecondForm)
	{
		switch (m_action)
		{
		case ACTION_SF_ATTACK:
			if (!IsAnimating())
			{
				ChangeSprite("EnemyLastBossIdle_Character");
				PlayAnimation(true);
				m_actionPhase = 1;
			}
			break;
		case ACTION_SF_CAST_SWORDS:
			if (m_actionPhase == 0 && !IsAnimating())
			{
				/* Swords from both room edges at the player's height. */
				if (m_levelScreen != nullptr && m_levelScreen->CurrentRoom() != nullptr)
				{
					Rectangle rb = m_levelScreen->CurrentRoom()->Bounds();
					Vector2 left(rb.X - 100, m_target->Y());
					Vector2 right(rb.Right() + 100, m_target->Y());
					m_levelScreen->FireProjectile("LastBossSwordProjectile_Sprite", left,
					                              Vector2(1, 0), EnemyEV::LastBoss_Advanced_ProjectileSpeed,
					                              (int) ProjectileDamage, Vector2(3.5f, 3.5f), false, this);
					m_levelScreen->FireProjectile("LastBossSwordProjectile_Sprite", right,
					                              Vector2(-1, 0), EnemyEV::LastBoss_Advanced_ProjectileSpeed,
					                              (int) ProjectileDamage, Vector2(3.5f, 3.5f), false, this);
				}
				m_actionPhase = 1;
			}
			else if (m_actionPhase == 1)
			{
				ChangeSprite("EnemyLastBossIdle_Character");
				PlayAnimation(true);
				m_actionPhase = 2;
			}
			break;
		case ACTION_SF_CAST_SPEARS:
			if (m_actionPhase == 0 && !IsAnimating())
			{
				/* Spears rise from the ground around the player. */
				for (int i = -1; i <= 2; i++)
					m_levelScreen->FireProjectile("LastBossSpearProjectile_Sprite",
					                              Vector2(m_target->X() + i * 120.0f, m_target->Y() + 200),
					                              Vector2(0, -1), EnemyEV::LastBoss_Advanced_ProjectileSpeed,
					                              (int) ProjectileDamage, Vector2(3.5f, 3.5f), false, this);
				m_actionPhase = 1;
			}
			else if (m_actionPhase == 1)
			{
				ChangeSprite("EnemyLastBossIdle_Character");
				PlayAnimation(true);
				m_actionPhase = 2;
			}
			break;
		default:
			break;
		}
		return;
	}

	switch (m_action)
	{
	case ACTION_JUMP:
	case ACTION_MOVE_ATTACK:
		if (m_isTouchingGround && m_action == ACTION_JUMP)
		{
			CurrentSpeed(0);
			ChangeSprite("PlayerIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 1;
		}
		else if (m_action == ACTION_MOVE_ATTACK && m_actionTimer <= 0)
		{
			CurrentSpeed(0);
			ChangeSprite("PlayerIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 1;
		}
		break;
	case ACTION_THROW_DAGGER:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The dagger at the player (SpellDagger, line 743). */
			float angle = atan2f(m_target->Y() - Y(), m_target->X() - X());
			m_levelScreen->FireProjectile("SpellDagger_Sprite", Position(),
			                              Vector2(cosf(angle), sinf(angle)),
			                              DaggerProjectileSpeed, (int) ProjectileDamage,
			                              Vector2(3.5f, 3.5f), false, this);
			AnimationDelay(1 / 10.0f);
			ChangeSprite("PlayerIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1)
			m_actionPhase = 2;
		break;
	case ACTION_DASH:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The dash itself. */
			CurrentSpeed(Speed * 3.5f);
			m_actionPhase = 1;
			m_actionTimer = 0.35f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			CurrentSpeed(0);
			ChangeSprite("PlayerIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_LastBoss::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_LastBoss::RunBasicLogic()
{
	if (m_formKilled)
	{
		/* The transition frame: nothing this step. */
		m_formKilled = false;
		return;
	}

	if (m_inSecondForm)
	{
		/* Line 789 (the second form weights, mapped onto the four
		 * ported actions). */
		static const Action acts[] = { ACTION_SF_WALK, ACTION_SF_ATTACK, ACTION_SF_CAST_SWORDS, ACTION_SF_CAST_SPEARS };
		switch (State)
		{
		case STATE_MELEE_ENGAGE:
		case STATE_PROJECTILE_ENGAGE:
		{
			static const int w[] = { 31, 26, 22, 21 };
			PickAction(w, 4, acts);
			break;
		}
		default:
			StartAction(ACTION_SF_WALK);
			break;
		}
		return;
	}

	/* Line 744 (the first form): walkTowardsLS, walkAwayLS, walkStopLS,
	 * jumpLS, moveAttackLS, throwShieldLS (weight 0), throwDaggerLS,
	 * dashLS. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_JUMP, ACTION_MOVE_ATTACK, ACTION_THROW_DAGGER, ACTION_DASH };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 0, 0, 0, 35, 35, 0, 30 };
		PickAction(w, 7, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 35, 0, 0, 25, 0, 20, 20 };
		PickAction(w, 7, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 40, 0, 0, 20, 0, 40, 0 };
		PickAction(w, 7, acts);
		break;
	}
	}
}

} // namespace RogueCastle
