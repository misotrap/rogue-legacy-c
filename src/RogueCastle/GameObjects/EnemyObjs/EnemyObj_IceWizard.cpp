/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_IceWizard.cs. */
#include "EnemyObj_IceWizard.h"
#include "../ProjectileObj.h"
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

EnemyObj_IceWizard::EnemyObj_IceWizard(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyWizardIdle_Character", target, difficulty)
{
	Type = EnemyType::IceWizard;
}

EnemyObj_IceWizard::~EnemyObj_IceWizard()
{
	ResetIceball();   /* the level screen owns the projectiles */
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_IceWizard.cs line 25), so they collapse into one. */
void EnemyObj_IceWizard::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::IceWizard_Miniboss_Name;
		MaxHealth = EnemyEV::IceWizard_Miniboss_MaxHealth;
		Damage = EnemyEV::IceWizard_Miniboss_Damage;
		XPValue = EnemyEV::IceWizard_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::IceWizard_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::IceWizard_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::IceWizard_Miniboss_DropChance;
		Speed = EnemyEV::IceWizard_Miniboss_Speed;
		TurnSpeed = EnemyEV::IceWizard_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::IceWizard_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::IceWizard_Miniboss_Jump;
		CooldownTime = EnemyEV::IceWizard_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::IceWizard_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::IceWizard_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::IceWizard_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::IceWizard_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::IceWizard_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::IceWizard_Miniboss_Scale.X, EnemyEV::IceWizard_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::IceWizard_Miniboss_ProjectileScale.X, EnemyEV::IceWizard_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::IceWizard_Miniboss_Tint.R, EnemyEV::IceWizard_Miniboss_Tint.G, EnemyEV::IceWizard_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::IceWizard_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::IceWizard_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::IceWizard_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::IceWizard_Miniboss_KnockBack.X, EnemyEV::IceWizard_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::IceWizard_Expert_Name;
		MaxHealth = EnemyEV::IceWizard_Expert_MaxHealth;
		Damage = EnemyEV::IceWizard_Expert_Damage;
		XPValue = EnemyEV::IceWizard_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::IceWizard_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::IceWizard_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::IceWizard_Expert_DropChance;
		Speed = EnemyEV::IceWizard_Expert_Speed;
		TurnSpeed = EnemyEV::IceWizard_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::IceWizard_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::IceWizard_Expert_Jump;
		CooldownTime = EnemyEV::IceWizard_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::IceWizard_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::IceWizard_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::IceWizard_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::IceWizard_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::IceWizard_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::IceWizard_Expert_Scale.X, EnemyEV::IceWizard_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::IceWizard_Expert_ProjectileScale.X, EnemyEV::IceWizard_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::IceWizard_Expert_Tint.R, EnemyEV::IceWizard_Expert_Tint.G, EnemyEV::IceWizard_Expert_Tint.B));
		MeleeRadius = EnemyEV::IceWizard_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::IceWizard_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::IceWizard_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::IceWizard_Expert_KnockBack.X, EnemyEV::IceWizard_Expert_KnockBack.Y);
		SpellProjectileCount = 14;   /* line 156 */
		break;
	case ADVANCED:
		Name = EnemyEV::IceWizard_Advanced_Name;
		MaxHealth = EnemyEV::IceWizard_Advanced_MaxHealth;
		Damage = EnemyEV::IceWizard_Advanced_Damage;
		XPValue = EnemyEV::IceWizard_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::IceWizard_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::IceWizard_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::IceWizard_Advanced_DropChance;
		Speed = EnemyEV::IceWizard_Advanced_Speed;
		TurnSpeed = EnemyEV::IceWizard_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::IceWizard_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::IceWizard_Advanced_Jump;
		CooldownTime = EnemyEV::IceWizard_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::IceWizard_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::IceWizard_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::IceWizard_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::IceWizard_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::IceWizard_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::IceWizard_Advanced_Scale.X, EnemyEV::IceWizard_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::IceWizard_Advanced_ProjectileScale.X, EnemyEV::IceWizard_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::IceWizard_Advanced_Tint.R, EnemyEV::IceWizard_Advanced_Tint.G, EnemyEV::IceWizard_Advanced_Tint.B));
		MeleeRadius = EnemyEV::IceWizard_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::IceWizard_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::IceWizard_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::IceWizard_Advanced_KnockBack.X, EnemyEV::IceWizard_Advanced_KnockBack.Y);
		SpellProjectileCount = 8;   /* line 113 */
		SpellDelay = 1.0f;
		m_spellOffset = Vector2(40, -130);
		break;
	default: /* BASIC */
		Name = EnemyEV::IceWizard_Basic_Name;
		MaxHealth = EnemyEV::IceWizard_Basic_MaxHealth;
		Damage = EnemyEV::IceWizard_Basic_Damage;
		XPValue = EnemyEV::IceWizard_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::IceWizard_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::IceWizard_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::IceWizard_Basic_DropChance;
		Speed = EnemyEV::IceWizard_Basic_Speed;
		TurnSpeed = EnemyEV::IceWizard_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::IceWizard_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::IceWizard_Basic_Jump;
		CooldownTime = EnemyEV::IceWizard_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::IceWizard_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::IceWizard_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::IceWizard_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::IceWizard_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::IceWizard_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::IceWizard_Basic_Scale.X, EnemyEV::IceWizard_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::IceWizard_Basic_ProjectileScale.X, EnemyEV::IceWizard_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::IceWizard_Basic_Tint.R, EnemyEV::IceWizard_Basic_Tint.G, EnemyEV::IceWizard_Basic_Tint.B));
		MeleeRadius = EnemyEV::IceWizard_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::IceWizard_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::IceWizard_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::IceWizard_Basic_KnockBack.X, EnemyEV::IceWizard_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_IceWizard::ResetState()
{
	/* A room transition interrupts any cast: release the held summon
	 * (it survives the transition, its owner may never update again). */
	ResetIceball();
	EnemyObj::ResetState();
}

void EnemyObj_IceWizard::InitializeEV()
{
	m_cooldownWeights[0] = 100; m_cooldownWeights[1] = 0; m_cooldownWeights[2] = 0;
	InitializeOne(DifficultyType);
}

void EnemyObj_IceWizard::SummonIceball()
{
	/* EnemyObj_IceWizard.cs line 393: the held iceball (the growth tween
	 * is visual only). */
	ResetIceball();
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 offset(m_spellOffset.X * facing, m_spellOffset.Y);
	m_iceballSummon = new ProjectileObj("WizardIceSpell_Sprite",
	                                     Position() + offset, Vector2(1, 0), 0,
	                                     (int) ProjectileDamage, ProjectileScale, false);
	m_iceballSummon->IsCollidableFlag = false;
	m_iceballSummon->Source = this;
	m_iceballSummon->Held = true;
	m_levelScreen->AddProjectile(m_iceballSummon);
}

void EnemyObj_IceWizard::ShatterIceball()
{
	/* ShatterIceball (line 423): shards all around, 360/count apart; the
	 * Expert's waves use random angles (line 489). */
	float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	Vector2 origin = Position() + Vector2(m_spellOffset.X * facing, m_spellOffset.Y);
	float angle = 0;
	float angleDiff = 360.0f / SpellProjectileCount;
	for (int i = 0; i < SpellProjectileCount; i++)
	{
		if (DifficultyType == EXPERT)
			angle = (float) (rand() % 361);
		float rad = angle * PI / 180.0f;
		Audio::PlaySound("Ice_Wizard_Attack_Glass");   /* EnemyObj_IceWizard.cs line 425 */
		m_levelScreen->FireProjectile("WizardIceProjectile_Sprite", origin,
		                              Vector2(cosf(rad), sinf(rad)), ProjectileSpeed,
		                              (int) ProjectileDamage, ProjectileScale, false, this, 0, true);   /* CollidesWithTerrain (the C# ProjectileData) */
		angle += angleDiff;
	}
}

void EnemyObj_IceWizard::ResetIceball()
{
	if (m_iceballSummon != nullptr)
	{
		m_iceballSummon->IsKilled = true;
		m_iceballSummon->Visible(false);
		m_iceballSummon->IsCollidableFlag = false;
		m_iceballSummon = nullptr;
	}
}

void EnemyObj_IceWizard::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_IceWizard::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_MOVE_TOWARDS:
	case ACTION_MOVE_AWAY:
		/* moveTowardsLS/moveAwayLS (lines 158, 162). */
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
		/* castSpellLS (line 171): channel, shatter, recover. */
		CurrentSpeed(0);
		ChangeSprite("EnemyWizardSpell_Character");
		PlayAnimation("Start", "BeforeSpell", false);
		m_shatterWavesLeft = DifficultyType == EXPERT ? 8 : 1;
		m_actionTimer = SpellDelay;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_IceWizard::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_CAST:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			if (m_iceballSummon == nullptr)
				SummonIceball();   /* the first wave summons, then shatters */
			ShatterIceball();
			ResetIceball();
			m_shatterWavesLeft--;
			if (m_shatterWavesLeft > 0)
				m_actionTimer = 0.135f;   /* the Expert's wave interval */
			else
			{
				m_actionPhase = 1;
				m_actionTimer = 0.5f;
			}
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

void EnemyObj_IceWizard::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* The held iceball follows the wizard. */
	if (m_iceballSummon != nullptr && !m_iceballSummon->IsAlive)
		m_iceballSummon = nullptr;
	if (m_iceballSummon != nullptr)
	{
		float facing = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
		m_iceballSummon->Position(Position() + Vector2(m_spellOffset.X * facing, m_spellOffset.Y));
	}
	UpdateAction();
}

void EnemyObj_IceWizard::RunBasicLogic()
{
	/* EnemyObj_IceWizard.cs line 283: like the fire wizard (the teleport
	 * LS has weight 0). */
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

void EnemyObj_IceWizard::Kill(bool giveXP)
{
	/* EnemyObj_IceWizard.cs Kill: the held spell goes with its caster */
	ResetIceball();
	EnemyObj::Kill(giveXP);
}

} // namespace RogueCastle

void RogueCastle::EnemyObj_IceWizard::StartCooldownMove(int move)
{
	static const Action moves[3] = { ACTION_MOVE_TOWARDS, ACTION_MOVE_AWAY, ACTION_STOP };
	StartAction(moves[move]);
}
