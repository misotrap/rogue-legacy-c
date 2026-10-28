/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_FireWizard.cs. */
#include "EnemyObj_FireWizard.h"
#include "../ProjectileObj.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_FireWizard::EnemyObj_FireWizard(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyWizardIdle_Character", target, difficulty)
{
	Type = EnemyType::FireWizard;
}

EnemyObj_FireWizard::~EnemyObj_FireWizard()
{
	ResetFireball();   /* the held fireball is owned by the level screen */
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_FireWizard.cs line 27), so they collapse into one. */
void EnemyObj_FireWizard::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::FireWizard_Miniboss_Name;
		MaxHealth = EnemyEV::FireWizard_Miniboss_MaxHealth;
		Damage = EnemyEV::FireWizard_Miniboss_Damage;
		XPValue = EnemyEV::FireWizard_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::FireWizard_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::FireWizard_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::FireWizard_Miniboss_DropChance;
		Speed = EnemyEV::FireWizard_Miniboss_Speed;
		TurnSpeed = EnemyEV::FireWizard_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::FireWizard_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::FireWizard_Miniboss_Jump;
		CooldownTime = EnemyEV::FireWizard_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::FireWizard_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::FireWizard_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::FireWizard_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::FireWizard_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::FireWizard_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::FireWizard_Miniboss_Scale.X, EnemyEV::FireWizard_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::FireWizard_Miniboss_ProjectileScale.X, EnemyEV::FireWizard_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::FireWizard_Miniboss_Tint.R, EnemyEV::FireWizard_Miniboss_Tint.G, EnemyEV::FireWizard_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::FireWizard_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::FireWizard_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::FireWizard_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::FireWizard_Miniboss_KnockBack.X, EnemyEV::FireWizard_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::FireWizard_Expert_Name;
		MaxHealth = EnemyEV::FireWizard_Expert_MaxHealth;
		Damage = EnemyEV::FireWizard_Expert_Damage;
		XPValue = EnemyEV::FireWizard_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::FireWizard_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::FireWizard_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::FireWizard_Expert_DropChance;
		Speed = EnemyEV::FireWizard_Expert_Speed;
		TurnSpeed = EnemyEV::FireWizard_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::FireWizard_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::FireWizard_Expert_Jump;
		CooldownTime = EnemyEV::FireWizard_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::FireWizard_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::FireWizard_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::FireWizard_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::FireWizard_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::FireWizard_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::FireWizard_Expert_Scale.X, EnemyEV::FireWizard_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::FireWizard_Expert_ProjectileScale.X, EnemyEV::FireWizard_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::FireWizard_Expert_Tint.R, EnemyEV::FireWizard_Expert_Tint.G, EnemyEV::FireWizard_Expert_Tint.B));
		MeleeRadius = EnemyEV::FireWizard_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::FireWizard_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::FireWizard_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::FireWizard_Expert_KnockBack.X, EnemyEV::FireWizard_Expert_KnockBack.Y);
		SpellInterval = 0.15f;   /* line 152 */
		break;
	case ADVANCED:
		Name = EnemyEV::FireWizard_Advanced_Name;
		MaxHealth = EnemyEV::FireWizard_Advanced_MaxHealth;
		Damage = EnemyEV::FireWizard_Advanced_Damage;
		XPValue = EnemyEV::FireWizard_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::FireWizard_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::FireWizard_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::FireWizard_Advanced_DropChance;
		Speed = EnemyEV::FireWizard_Advanced_Speed;
		TurnSpeed = EnemyEV::FireWizard_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::FireWizard_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::FireWizard_Advanced_Jump;
		CooldownTime = EnemyEV::FireWizard_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::FireWizard_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::FireWizard_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::FireWizard_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::FireWizard_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::FireWizard_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::FireWizard_Advanced_Scale.X, EnemyEV::FireWizard_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::FireWizard_Advanced_ProjectileScale.X, EnemyEV::FireWizard_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::FireWizard_Advanced_Tint.R, EnemyEV::FireWizard_Advanced_Tint.G, EnemyEV::FireWizard_Advanced_Tint.B));
		MeleeRadius = EnemyEV::FireWizard_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::FireWizard_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::FireWizard_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::FireWizard_Advanced_KnockBack.X, EnemyEV::FireWizard_Advanced_KnockBack.Y);
		SpellDelay = 1.0f;   /* line 111 */
		SpellInterval = 1.0f;
		break;
	default: /* BASIC */
		Name = EnemyEV::FireWizard_Basic_Name;
		MaxHealth = EnemyEV::FireWizard_Basic_MaxHealth;
		Damage = EnemyEV::FireWizard_Basic_Damage;
		XPValue = EnemyEV::FireWizard_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::FireWizard_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::FireWizard_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::FireWizard_Basic_DropChance;
		Speed = EnemyEV::FireWizard_Basic_Speed;
		TurnSpeed = EnemyEV::FireWizard_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::FireWizard_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::FireWizard_Basic_Jump;
		CooldownTime = EnemyEV::FireWizard_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::FireWizard_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::FireWizard_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::FireWizard_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::FireWizard_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::FireWizard_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::FireWizard_Basic_Scale.X, EnemyEV::FireWizard_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::FireWizard_Basic_ProjectileScale.X, EnemyEV::FireWizard_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::FireWizard_Basic_Tint.R, EnemyEV::FireWizard_Basic_Tint.G, EnemyEV::FireWizard_Basic_Tint.B));
		MeleeRadius = EnemyEV::FireWizard_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::FireWizard_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::FireWizard_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::FireWizard_Basic_KnockBack.X, EnemyEV::FireWizard_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_FireWizard::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_FireWizard::SummonFireball()
{
	/* EnemyObj_FireWizard.cs line 434: the held fireball (the original
	 * tweens its opacity and scale; the port shows it full size). */
	ResetFireball();
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 offset(m_spellOffset.X * facing, m_spellOffset.Y);
	m_fireballSummon = new ProjectileObj("WizardFireballProjectile_Sprite",
	                                     Position() + offset, Vector2(1, 0), 0,
	                                     (int) ProjectileDamage, ProjectileScale, false);
	m_fireballSummon->IsCollidableFlag = false;   /* held, not live yet */
	m_fireballSummon->Source = this;
	m_levelScreen->AddProjectile(m_fireballSummon);
}

void EnemyObj_FireWizard::CastFireball()
{
	/* Line 400: a fireball at the player from the spell offset; the
	 * Advanced one is angled randomly, the Expert fires the ghost
	 * projectile. */
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 origin = Position() + Vector2(m_spellOffset.X * facing, m_spellOffset.Y);
	float angle = atan2f(m_target->Y() - origin.Y, m_target->X() - origin.X) * 180.0f / PI;
	if (DifficultyType == ADVANCED)
		angle += (float) (rand() % 51 - 25);
	float rad = angle * PI / 180.0f;
	const char *sprite = DifficultyType == EXPERT ? "GhostBossProjectile_Sprite"
	                                              : "WizardFireballProjectile_Sprite";
	m_levelScreen->FireProjectile(sprite, origin, Vector2(cosf(rad), sinf(rad)),
	                              ProjectileSpeed, (int) ProjectileDamage,
	                              ProjectileScale, false, this);
}

void EnemyObj_FireWizard::ResetFireball()
{
	/* Line 424 (ResetFireball): the level screen frees killed
	 * projectiles on its next update. */
	if (m_fireballSummon != nullptr)
	{
		m_fireballSummon->IsKilled = true;
		m_fireballSummon->Visible(false);
		m_fireballSummon->IsCollidableFlag = false;
		m_fireballSummon = nullptr;
	}
}

void EnemyObj_FireWizard::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_FireWizard::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_MOVE_TOWARDS:
	case ACTION_MOVE_AWAY:
		/* moveTowardsLS/moveAwayLS (lines 158, 162): hover towards or
		 * away from the player. */
		ChangeSprite("EnemyWizardIdle_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_MOVE_TOWARDS ? 1 : -1)
		                               : (action == ACTION_MOVE_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = MoveDuration;
		break;
	case ACTION_STOP:
		/* walkStopLS (line 166). */
		ChangeSprite("EnemyWizardIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = 0.5f;
		break;
	case ACTION_CAST:
		/* castSpellLS (line 171): channel, then three fireballs. */
		CurrentSpeed(0);
		ChangeSprite("EnemyWizardSpell_Character");
		PlayAnimation("Start", "BeforeSpell", false);
		m_castsLeft = 3;
		m_actionTimer = SpellDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_FireWizard::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_CAST:
		if (m_castsLeft > 0 && m_actionTimer <= 0)
		{
			if (m_castsLeft == 3)
				SummonFireball();
			if (m_castsLeft == 1)
				ResetFireball();   /* the summon flies with the last cast */
			CastFireball();
			m_castsLeft--;
			m_actionTimer = m_castsLeft > 0 ? SpellInterval : 0.5f;
			if (m_castsLeft == 0)
				m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyWizardIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_FireWizard::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* The held fireball follows the wizard's hand (line 381). */
	if (m_fireballSummon != nullptr)
	{
		float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
		m_fireballSummon->Position(Position() + Vector2(m_spellOffset.X * facing, m_spellOffset.Y));
	}
	UpdateAction();
}

void EnemyObj_FireWizard::RunBasicLogic()
{
	/* EnemyObj_FireWizard.cs line 263. moveTowardsLS, moveAwayLS,
	 * walkStopLS, castSpellLS (the teleport LS has weight 0). */
	static const Action acts[] = { ACTION_MOVE_TOWARDS, ACTION_MOVE_AWAY, ACTION_STOP, ACTION_CAST };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 40, 0, 0, 60 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 100, 0, 0, 0 };
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

} // namespace RogueCastle
