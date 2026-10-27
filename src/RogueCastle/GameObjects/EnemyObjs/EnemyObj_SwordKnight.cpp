/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SwordKnight.cs. */
#include "EnemyObj_SwordKnight.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_SwordKnight::EnemyObj_SwordKnight(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySwordKnightIdle_Character", target, difficulty)
{
	Type = EnemyType::SwordKnight;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_SwordKnight.cs line 22), so they collapse into one. */
void EnemyObj_SwordKnight::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::SwordKnight_Miniboss_Name;
		MaxHealth = EnemyEV::SwordKnight_Miniboss_MaxHealth;
		Damage = EnemyEV::SwordKnight_Miniboss_Damage;
		XPValue = EnemyEV::SwordKnight_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::SwordKnight_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SwordKnight_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::SwordKnight_Miniboss_DropChance;
		Speed = EnemyEV::SwordKnight_Miniboss_Speed;
		TurnSpeed = EnemyEV::SwordKnight_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::SwordKnight_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::SwordKnight_Miniboss_Jump;
		CooldownTime = EnemyEV::SwordKnight_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::SwordKnight_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SwordKnight_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SwordKnight_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SwordKnight_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SwordKnight_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::SwordKnight_Miniboss_Scale.X, EnemyEV::SwordKnight_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SwordKnight_Miniboss_ProjectileScale.X, EnemyEV::SwordKnight_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SwordKnight_Miniboss_Tint.R, EnemyEV::SwordKnight_Miniboss_Tint.G, EnemyEV::SwordKnight_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::SwordKnight_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::SwordKnight_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::SwordKnight_Miniboss_EngageRadius;
		SlashDelay = 1.05f;   /* EnemyObj_SwordKnight.cs line 66 */
		break;
	case EXPERT:
		Name = EnemyEV::SwordKnight_Expert_Name;
		MaxHealth = EnemyEV::SwordKnight_Expert_MaxHealth;
		Damage = EnemyEV::SwordKnight_Expert_Damage;
		XPValue = EnemyEV::SwordKnight_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::SwordKnight_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SwordKnight_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::SwordKnight_Expert_DropChance;
		Speed = EnemyEV::SwordKnight_Expert_Speed;
		TurnSpeed = EnemyEV::SwordKnight_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::SwordKnight_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::SwordKnight_Expert_Jump;
		CooldownTime = EnemyEV::SwordKnight_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::SwordKnight_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SwordKnight_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SwordKnight_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SwordKnight_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SwordKnight_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::SwordKnight_Expert_Scale.X, EnemyEV::SwordKnight_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SwordKnight_Expert_ProjectileScale.X, EnemyEV::SwordKnight_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SwordKnight_Expert_Tint.R, EnemyEV::SwordKnight_Expert_Tint.G, EnemyEV::SwordKnight_Expert_Tint.B));
		MeleeRadius = EnemyEV::SwordKnight_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::SwordKnight_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::SwordKnight_Expert_EngageRadius;
		SlashDelay = 0.25f;
		TripleAttackSpeed = 500;
		break;
	case ADVANCED:
		Name = EnemyEV::SwordKnight_Advanced_Name;
		MaxHealth = EnemyEV::SwordKnight_Advanced_MaxHealth;
		Damage = EnemyEV::SwordKnight_Advanced_Damage;
		XPValue = EnemyEV::SwordKnight_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::SwordKnight_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SwordKnight_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::SwordKnight_Advanced_DropChance;
		Speed = EnemyEV::SwordKnight_Advanced_Speed;
		TurnSpeed = EnemyEV::SwordKnight_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::SwordKnight_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::SwordKnight_Advanced_Jump;
		CooldownTime = EnemyEV::SwordKnight_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::SwordKnight_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SwordKnight_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SwordKnight_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SwordKnight_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SwordKnight_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::SwordKnight_Advanced_Scale.X, EnemyEV::SwordKnight_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SwordKnight_Advanced_ProjectileScale.X, EnemyEV::SwordKnight_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SwordKnight_Advanced_Tint.R, EnemyEV::SwordKnight_Advanced_Tint.G, EnemyEV::SwordKnight_Advanced_Tint.B));
		MeleeRadius = EnemyEV::SwordKnight_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::SwordKnight_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::SwordKnight_Advanced_EngageRadius;
		SlashDelay = 0.25f;
		break;
	default: /* BASIC */
		Name = EnemyEV::SwordKnight_Basic_Name;
		MaxHealth = EnemyEV::SwordKnight_Basic_MaxHealth;
		Damage = EnemyEV::SwordKnight_Basic_Damage;
		XPValue = EnemyEV::SwordKnight_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::SwordKnight_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::SwordKnight_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::SwordKnight_Basic_DropChance;
		Speed = EnemyEV::SwordKnight_Basic_Speed;
		TurnSpeed = EnemyEV::SwordKnight_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::SwordKnight_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::SwordKnight_Basic_Jump;
		CooldownTime = EnemyEV::SwordKnight_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::SwordKnight_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::SwordKnight_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::SwordKnight_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::SwordKnight_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::SwordKnight_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::SwordKnight_Basic_Scale.X, EnemyEV::SwordKnight_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::SwordKnight_Basic_ProjectileScale.X, EnemyEV::SwordKnight_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::SwordKnight_Basic_Tint.R, EnemyEV::SwordKnight_Basic_Tint.G, EnemyEV::SwordKnight_Basic_Tint.B));
		MeleeRadius = EnemyEV::SwordKnight_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::SwordKnight_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::SwordKnight_Basic_EngageRadius;
		SlashDelay = 0.25f;
		break;
	}
	ProjectileDamage = Damage;
	KnockBack = Vector2(EnemyEV::SwordKnight_Basic_KnockBack.X, EnemyEV::SwordKnight_Basic_KnockBack.Y);
}

void EnemyObj_SwordKnight::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_SwordKnight::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_SwordKnight::FireWave()
{
	/* The attackLS FireProjectileLogicAction: a wave from the sword
	 * (SourceAnchor (60, 0)) in the facing direction. */
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 heading(facing, 0);
	m_levelScreen->FireProjectile("EnemySpearKnightWave_Sprite",
	                              Position() + Vector2(60.0f * facing, 0), heading,
	                              ProjectileSpeed, (int) ProjectileDamage,
	                              ProjectileScale, false, this);
}

void EnemyObj_SwordKnight::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;
	m_attackFiresWave = false;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS/walkAwayLS (lines 155, 161). */
		ChangeSprite("EnemySwordKnightWalk_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                               : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = 1.0f;
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 165). */
		ChangeSprite("EnemySwordKnightIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = 0.5f;
		break;
	case ACTION_ATTACK:
	case ACTION_ATTACK_TRIPLE:
		/* attackLS (line 169) / attackTripleLS (line 177): face the
		 * player and wind up. */
		CurrentSpeed(0);
		HeadingX(m_target->X() >= X() ? 1 : -1);
		ChangeSprite("EnemySwordKnightAttack_Character");
		PlayAnimation("Start", "Windup", false);
		m_actionTimer = action == ACTION_ATTACK ? SlashDelay : SlashTripleDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		/* Advanced/Expert slashes send the wave (lines 223, 254); the
		 * Expert triple fires it after the dash (line 245). */
		if (action == ACTION_ATTACK)
			m_attackFiresWave = DifficultyType == ADVANCED || DifficultyType == EXPERT;
		else
			m_attackFiresWave = DifficultyType == EXPERT;
		m_endPhase = action == ACTION_ATTACK ? 2 : 5;
		break;
	default:
		break;
	}
}

void EnemyObj_SwordKnight::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_ATTACK:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			if (m_attackFiresWave)
				FireWave();
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ChangeSprite("EnemySwordKnightIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	case ACTION_ATTACK_TRIPLE:
		/* attackTripleLS (line 177): wind up, dash with three fast
		 * slashes, then the wave for the Expert (line 245). */
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			CurrentSpeed(TripleAttackSpeed);
			AnimationDelay(1 / 60.0f);
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase >= 1 && m_actionPhase < 4 && !IsAnimating())
		{
			PlayAnimation(m_actionPhase % 2 == 1 ? "Start" : "Attack", "End", false);
			m_actionPhase++;
		}
		else if (m_actionPhase == 4 && !IsAnimating())
		{
			CurrentSpeed(0);
			AnimationDelay(1 / EnemyEV::SwordKnight_Advanced_AnimationDelay);
			if (m_attackFiresWave)
				FireWave();
			ChangeSprite("EnemySwordKnightIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 5;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_SwordKnight::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_SwordKnight::RunBasicLogic()
{
	/* EnemyObj_SwordKnight.cs line 300.
	 * walkTowardsLS, walkAwayLS, walkStopLS, attackLS. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP, ACTION_ATTACK };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 0, 0, 0, 100 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 15, 15, 70, 0 };
		PickAction(w, 4, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 0, 0, 100, 0 };
		PickAction(w, 4, acts);
		break;
	}
	}
}

void EnemyObj_SwordKnight::RunAdvancedLogic()
{
	/* Line 322: the Advanced slash sends the wave; MELEE mixes in the
	 * triple attack (65/35, line 328). */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_ATTACK, ACTION_ATTACK_TRIPLE };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 0, 0, 0, 65, 35 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 60, 20, 20, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	default:
	{
		static const int w[] = { 0, 0, 100, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_SwordKnight::RunExpertLogic()
{
	/* Line 343: the Expert triple dashes and then fires the wave
	 * (62/38, line 348). */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_ATTACK, ACTION_ATTACK_TRIPLE };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	{
		static const int w[] = { 0, 0, 0, 62, 38 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 60, 20, 20, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	default:
	{
		static const int w[] = { 0, 0, 100, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

void EnemyObj_SwordKnight::RunMinibossLogic()
{
	/* Line 365: the miniboss only walks towards the player. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP, ACTION_ATTACK };
	static const int w[] = { 100, 0, 0, 0 };
	PickAction(w, 4, acts);
}

} // namespace RogueCastle
