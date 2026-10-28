/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_EarthWizard.cs. */
#include "EnemyObj_EarthWizard.h"
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

EnemyObj_EarthWizard::EnemyObj_EarthWizard(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyWizardIdle_Character", target, difficulty)
{
	Type = EnemyType::EarthWizard;
}

EnemyObj_EarthWizard::~EnemyObj_EarthWizard()
{
	CancelEarthSpell();
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_EarthWizard.cs line 27), so they collapse into one. */
void EnemyObj_EarthWizard::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::EarthWizard_Miniboss_Name;
		MaxHealth = EnemyEV::EarthWizard_Miniboss_MaxHealth;
		Damage = EnemyEV::EarthWizard_Miniboss_Damage;
		XPValue = EnemyEV::EarthWizard_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::EarthWizard_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::EarthWizard_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::EarthWizard_Miniboss_DropChance;
		Speed = EnemyEV::EarthWizard_Miniboss_Speed;
		TurnSpeed = EnemyEV::EarthWizard_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::EarthWizard_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::EarthWizard_Miniboss_Jump;
		CooldownTime = EnemyEV::EarthWizard_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::EarthWizard_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::EarthWizard_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::EarthWizard_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::EarthWizard_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::EarthWizard_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::EarthWizard_Miniboss_Scale.X, EnemyEV::EarthWizard_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::EarthWizard_Miniboss_ProjectileScale.X, EnemyEV::EarthWizard_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::EarthWizard_Miniboss_Tint.R, EnemyEV::EarthWizard_Miniboss_Tint.G, EnemyEV::EarthWizard_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::EarthWizard_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::EarthWizard_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::EarthWizard_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::EarthWizard_Miniboss_KnockBack.X, EnemyEV::EarthWizard_Miniboss_KnockBack.Y);
		SpellDelay = 0.85f;   /* line 89 */
		SpellDuration = 2.0f;
		break;
	case EXPERT:
		Name = EnemyEV::EarthWizard_Expert_Name;
		MaxHealth = EnemyEV::EarthWizard_Expert_MaxHealth;
		Damage = EnemyEV::EarthWizard_Expert_Damage;
		XPValue = EnemyEV::EarthWizard_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::EarthWizard_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::EarthWizard_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::EarthWizard_Expert_DropChance;
		Speed = EnemyEV::EarthWizard_Expert_Speed;
		TurnSpeed = EnemyEV::EarthWizard_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::EarthWizard_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::EarthWizard_Expert_Jump;
		CooldownTime = EnemyEV::EarthWizard_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::EarthWizard_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::EarthWizard_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::EarthWizard_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::EarthWizard_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::EarthWizard_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::EarthWizard_Expert_Scale.X, EnemyEV::EarthWizard_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::EarthWizard_Expert_ProjectileScale.X, EnemyEV::EarthWizard_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::EarthWizard_Expert_Tint.R, EnemyEV::EarthWizard_Expert_Tint.G, EnemyEV::EarthWizard_Expert_Tint.B));
		MeleeRadius = EnemyEV::EarthWizard_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::EarthWizard_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::EarthWizard_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::EarthWizard_Expert_KnockBack.X, EnemyEV::EarthWizard_Expert_KnockBack.Y);
		SpellDelay = 0.5f;   /* line 172 */
		SpellDuration = 1.0f;
		break;
	case ADVANCED:
		Name = EnemyEV::EarthWizard_Advanced_Name;
		MaxHealth = EnemyEV::EarthWizard_Advanced_MaxHealth;
		Damage = EnemyEV::EarthWizard_Advanced_Damage;
		XPValue = EnemyEV::EarthWizard_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::EarthWizard_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::EarthWizard_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::EarthWizard_Advanced_DropChance;
		Speed = EnemyEV::EarthWizard_Advanced_Speed;
		TurnSpeed = EnemyEV::EarthWizard_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::EarthWizard_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::EarthWizard_Advanced_Jump;
		CooldownTime = EnemyEV::EarthWizard_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::EarthWizard_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::EarthWizard_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::EarthWizard_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::EarthWizard_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::EarthWizard_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::EarthWizard_Advanced_Scale.X, EnemyEV::EarthWizard_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::EarthWizard_Advanced_ProjectileScale.X, EnemyEV::EarthWizard_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::EarthWizard_Advanced_Tint.R, EnemyEV::EarthWizard_Advanced_Tint.G, EnemyEV::EarthWizard_Advanced_Tint.B));
		MeleeRadius = EnemyEV::EarthWizard_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::EarthWizard_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::EarthWizard_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::EarthWizard_Advanced_KnockBack.X, EnemyEV::EarthWizard_Advanced_KnockBack.Y);
		SpellDelay = 0.7f;   /* line 131 */
		SpellDuration = 3.5f;
		break;
	default: /* BASIC */
		Name = EnemyEV::EarthWizard_Basic_Name;
		MaxHealth = EnemyEV::EarthWizard_Basic_MaxHealth;
		Damage = EnemyEV::EarthWizard_Basic_Damage;
		XPValue = EnemyEV::EarthWizard_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::EarthWizard_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::EarthWizard_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::EarthWizard_Basic_DropChance;
		Speed = EnemyEV::EarthWizard_Basic_Speed;
		TurnSpeed = EnemyEV::EarthWizard_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::EarthWizard_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::EarthWizard_Basic_Jump;
		CooldownTime = EnemyEV::EarthWizard_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::EarthWizard_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::EarthWizard_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::EarthWizard_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::EarthWizard_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::EarthWizard_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::EarthWizard_Basic_Scale.X, EnemyEV::EarthWizard_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::EarthWizard_Basic_ProjectileScale.X, EnemyEV::EarthWizard_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::EarthWizard_Basic_Tint.R, EnemyEV::EarthWizard_Basic_Tint.G, EnemyEV::EarthWizard_Basic_Tint.B));
		MeleeRadius = EnemyEV::EarthWizard_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::EarthWizard_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::EarthWizard_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::EarthWizard_Basic_KnockBack.X, EnemyEV::EarthWizard_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_EarthWizard::ResetState()
{
	/* A room transition interrupts any cast: release the held summon
	 * (it survives the transition, its owner may never update again). */
	CancelEarthSpell();
	EnemyObj::ResetState();
}

void EnemyObj_EarthWizard::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_EarthWizard::CancelEarthSpell()
{
	/* CancelEarthSpell (line 456): free the channel marker. */
	if (m_earthSummon != nullptr)
	{
		m_earthSummon->IsKilled = true;
		m_earthSummon->Visible(false);
		m_earthSummon->IsCollidableFlag = false;
		m_earthSummon = nullptr;
	}
}

void EnemyObj_EarthWizard::CastFireball()
{
	/* The miniboss fire cast (line 213): fireballs at the player from
	 * the spell offset. */
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 origin = Position() + Vector2(m_spellOffset.X * facing, m_spellOffset.Y);
	float angle = atan2f(m_target->Y() - origin.Y, m_target->X() - origin.X) * 180.0f / PI;
	float rad = angle * PI / 180.0f;
	m_levelScreen->FireProjectile("WizardFireballProjectile_Sprite", origin,
	                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
	                              (int) ProjectileDamage, ProjectileScale, false, this);
}

void EnemyObj_EarthWizard::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_EarthWizard::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_MOVE_TOWARDS:
	case ACTION_MOVE_AWAY:
		/* moveTowardsLS/moveAwayLS (lines 178, 181). */
		ChangeSprite("EnemyWizardIdle_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_MOVE_TOWARDS ? 1 : -1)
		                               : (action == ACTION_MOVE_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = MoveDuration;
		break;
	case ACTION_STOP:
		/* walkStopLS (line 184). */
		ChangeSprite("EnemyWizardIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = 0.5f;
		break;
	case ACTION_CAST_EARTH:
		/* castSpellLS (line 188): channel under the player, then the
		 * spike erupts there for SpellDuration. */
		CurrentSpeed(0);
		ChangeSprite("EnemyWizardSpell_Character");
		PlayAnimation("Start", "BeforeSpell", false);
		/* CastEarthSpellIn (line 475): the channel marker on the
		 * player's feet (it follows the player, line 567). */
		CancelEarthSpell();
		m_earthSummon = new ProjectileObj("WizardEarthSpellCast_Sprite",
		                                  m_target->Position(), Vector2(1, 0), 0,
		                                  (int) ProjectileDamage, ProjectileScale, false);
		m_earthSummon->IsCollidableFlag = false;
		m_earthSummon->Source = this;
	m_earthSummon->Held = true;
		m_levelScreen->AddProjectile(m_earthSummon);
		m_actionTimer = 0.5f;   /* CastEarthSpellIn, then CastEarthSpellOut */
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 3;
		break;
	case ACTION_CAST_FIRE:
		/* castMiniBossFireSpellLS (line 206): three fireballs. */
		CurrentSpeed(0);
		ChangeSprite("EnemyWizardSpell_Character");
		PlayAnimation("Start", "BeforeSpell", false);
		m_fireCastsLeft = 3;
		m_actionTimer = SpellFireDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_EarthWizard::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_CAST_EARTH:
		/* The channel marker tracks the player's feet (line 567); the
		 * level screen may have freed it (room transition). */
		if (m_earthSummon != nullptr && !m_earthSummon->IsAlive)
			m_earthSummon = nullptr;
		if (m_earthSummon != nullptr)
			m_earthSummon->Position(m_target->Position());
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* CastEarthSpellOut: the marker grows. */
			m_actionPhase = 1;
			m_actionTimer = SpellDelay;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			/* CastEarthSpell (line 549): the spike at the player's
			 * position, alive for SpellDuration (the C# cancels it with
			 * a delayed RunFunction). */
			CancelEarthSpell();
			ProjectileObj *spike = new ProjectileObj("WizardEarthSpell_Sprite",
			                                         m_target->Position(), Vector2(1, 0), 0,
			                                         (int) ProjectileDamage, ProjectileScale, false);
			spike->LifeSpan = SpellDuration;
			spike->Source = this;
			m_levelScreen->AddProjectile(spike);
			m_actionPhase = 2;
			m_actionTimer = 0.5f;
		}
		else if (m_actionPhase == 2 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyWizardIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 3;
		}
		break;
	case ACTION_CAST_FIRE:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			CastFireball();
			m_fireCastsLeft--;
			m_actionTimer = SpellFireInterval;
			if (m_fireCastsLeft == 0)
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

void EnemyObj_EarthWizard::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_EarthWizard::RunBasicLogic()
{
	/* EnemyObj_EarthWizard.cs line 328 (the teleport LS has weight 0,
	 * like the other wizards). */
	static const Action acts[] = { ACTION_MOVE_TOWARDS, ACTION_MOVE_AWAY, ACTION_STOP, ACTION_CAST_EARTH };
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

void EnemyObj_EarthWizard::RunMinibossLogic()
{
	/* Line 373: move, earth spike, fireballs, ice shards (the ice cast
	 * of the original is not ported). */
	static const Action acts[] = { ACTION_MOVE_TOWARDS, ACTION_MOVE_AWAY, ACTION_STOP,
	                               ACTION_CAST_EARTH, ACTION_CAST_FIRE };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 34, 0, 0, 33, 33 };
		PickAction(w, 5, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 100, 0, 0, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 60, 10, 30, 0, 0 };
		PickAction(w, 5, acts);
		break;
	}
	}
}

} // namespace RogueCastle
