/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Energon.cs. */
#include "EnemyObj_Energon.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../ProjectileObj.h"
#include "../../../Audio/Audio.h"
#include "../../../DS2DEngine/Consts.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Energon::EnemyObj_Energon(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyEnergonIdle_Character", target, difficulty)
{
	Type = EnemyType::Energon;
	m_shield = new DS2DEngine::SpriteObj("EnergonSwordShield_Sprite");
	m_shield->AnimationDelay(1 / 10.0f);
	m_shield->PlayAnimation(true);
	m_shield->Opacity(0.5f);
	m_shield->Scale(Vector2(1.2f, 1.2f));
}

EnemyObj_Energon::~EnemyObj_Energon()
{
	delete m_shield;
}

void EnemyObj_Energon::SwitchType(unsigned char type)
{
	m_currentAttackType = type;
	static const char *const SHIELDS[3] = { "EnergonSwordShield_Sprite", "EnergonShieldShield_Sprite", "EnergonDownSwordShield_Sprite" };
	m_shield->ChangeSprite(SHIELDS[type]);
	m_shield->PlayAnimation(true);
}

void EnemyObj_Energon::Draw(DS2DEngine::Camera2D *camera)
{
	EnemyObj::Draw(camera);
	if (IsKilled || !Visible()) return;
	m_shield->Position(Position());
	m_shield->Flip(Flip());
	m_shield->Draw(camera);
}

void EnemyObj_Energon::Kill(bool giveXP)
{
	if (m_levelScreen != nullptr) m_levelScreen->KillProjectilesFrom(this);
	EnemyObj::Kill(giveXP);
}

void EnemyObj_Energon::CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
                                         int collisionResponseType)
{
	/* EnemyObj_Energon.cs CollisionResponse */
	DS2DEngine::IPhysicsObj *other = otherBox->PhysicsParent();
	if (collisionResponseType == Consts::COLLISIONRESPONSE_FIRSTBOXHIT && other != nullptr && m_target != nullptr)
	{
		if (other == (DS2DEngine::IPhysicsObj *) m_target)
		{
			// The sword bounces off: the hero is pushed back, no damage
			if (m_invincibleCounter <= 0)
			{
				Rectangle b = m_target->Bounds();
				float kb = m_target->EnemyKnockBack.X;
				m_target->AccelerationX(b.X + b.Width / 2 < X() ? -kb : kb);
				m_target->AccelerationY(-m_target->EnemyKnockBack.Y);
				Audio::PlaySound("Player_Block");
				m_invincibleCounter = InvincibilityTime;
				Blink(Color(173, 216, 230), 0.1f);
			}
			return;
		}
		if (other->IsSpellProjectile())
		{
			ProjectileObj *proj = static_cast<ProjectileObj *>(other);
			if (proj->EnergonType >= 0)
			{
				// Its own shot sent back: it hurts only if the shield matches
				if (proj->IsKilled) return;
				proj->KillProjectile();
				Rectangle a = thisBox->AbsRect();
				if (proj->EnergonType == m_currentAttackType)
					HitEnemy(proj->DealsDamage(), Vector2((float) a.X, (float) a.Y), true);
				else
					Audio::PlaySound("Player_Block");
				return;
			}
		}
	}
	EnemyObj::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Energon.cs line 22), so they collapse into one. */
void EnemyObj_Energon::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Energon_Miniboss_Name;
		MaxHealth = EnemyEV::Energon_Miniboss_MaxHealth;
		Damage = EnemyEV::Energon_Miniboss_Damage;
		XPValue = EnemyEV::Energon_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Energon_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Energon_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Energon_Miniboss_DropChance;
		Speed = EnemyEV::Energon_Miniboss_Speed;
		TurnSpeed = EnemyEV::Energon_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Energon_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Energon_Miniboss_Jump;
		CooldownTime = EnemyEV::Energon_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Energon_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Energon_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Energon_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Energon_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Energon_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Energon_Miniboss_Scale.X, EnemyEV::Energon_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Energon_Miniboss_ProjectileScale.X, EnemyEV::Energon_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Energon_Miniboss_Tint.R, EnemyEV::Energon_Miniboss_Tint.G, EnemyEV::Energon_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Energon_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Energon_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Energon_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Energon_Miniboss_KnockBack.X, EnemyEV::Energon_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Energon_Expert_Name;
		MaxHealth = EnemyEV::Energon_Expert_MaxHealth;
		Damage = EnemyEV::Energon_Expert_Damage;
		XPValue = EnemyEV::Energon_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Energon_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Energon_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Energon_Expert_DropChance;
		Speed = EnemyEV::Energon_Expert_Speed;
		TurnSpeed = EnemyEV::Energon_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Energon_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Energon_Expert_Jump;
		CooldownTime = EnemyEV::Energon_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Energon_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Energon_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Energon_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Energon_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Energon_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Energon_Expert_Scale.X, EnemyEV::Energon_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Energon_Expert_ProjectileScale.X, EnemyEV::Energon_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Energon_Expert_Tint.R, EnemyEV::Energon_Expert_Tint.G, EnemyEV::Energon_Expert_Tint.B));
		MeleeRadius = EnemyEV::Energon_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Energon_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Energon_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Energon_Expert_KnockBack.X, EnemyEV::Energon_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Energon_Advanced_Name;
		MaxHealth = EnemyEV::Energon_Advanced_MaxHealth;
		Damage = EnemyEV::Energon_Advanced_Damage;
		XPValue = EnemyEV::Energon_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Energon_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Energon_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Energon_Advanced_DropChance;
		Speed = EnemyEV::Energon_Advanced_Speed;
		TurnSpeed = EnemyEV::Energon_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Energon_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Energon_Advanced_Jump;
		CooldownTime = EnemyEV::Energon_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Energon_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Energon_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Energon_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Energon_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Energon_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Energon_Advanced_Scale.X, EnemyEV::Energon_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Energon_Advanced_ProjectileScale.X, EnemyEV::Energon_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Energon_Advanced_Tint.R, EnemyEV::Energon_Advanced_Tint.G, EnemyEV::Energon_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Energon_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Energon_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Energon_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Energon_Advanced_KnockBack.X, EnemyEV::Energon_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Energon_Basic_Name;
		MaxHealth = EnemyEV::Energon_Basic_MaxHealth;
		Damage = EnemyEV::Energon_Basic_Damage;
		XPValue = EnemyEV::Energon_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Energon_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Energon_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Energon_Basic_DropChance;
		Speed = EnemyEV::Energon_Basic_Speed;
		TurnSpeed = EnemyEV::Energon_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Energon_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Energon_Basic_Jump;
		CooldownTime = EnemyEV::Energon_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Energon_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Energon_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Energon_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Energon_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Energon_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Energon_Basic_Scale.X, EnemyEV::Energon_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Energon_Basic_ProjectileScale.X, EnemyEV::Energon_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Energon_Basic_Tint.R, EnemyEV::Energon_Basic_Tint.G, EnemyEV::Energon_Basic_Tint.B));
		MeleeRadius = EnemyEV::Energon_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Energon_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Energon_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Energon_Basic_KnockBack.X, EnemyEV::Energon_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Energon::InitializeEV()
{
	m_cooldownWeights[0] = 0; m_cooldownWeights[1] = 0; m_cooldownWeights[2] = 100;
	InitializeOne(DifficultyType);
}

void EnemyObj_Energon::SwitchRandomType()
{
	/* SwitchRandomType (line 334): a type different from the current
	 * one (the shield sprite change is visual only). */
	unsigned char type = m_currentAttackType;
	while (type == m_currentAttackType)
		type = (unsigned char) (rand() % 3);
	SwitchType(type);
}

void EnemyObj_Energon::FireProjectile()
{
	/* FireProjectile (line 297): the weapon ghost of the current type,
	 * homing at the hero (EnergonProjectileObj: ChaseTarget, TurnSpeed 1) */
	const char *sprite = "EnergonSwordProjectile_Sprite";
	switch (m_currentAttackType)
	{
	case TYPE_SHIELD: sprite = "EnergonShieldProjectile_Sprite"; break;
	case TYPE_DOWNSWORD: sprite = "EnergonDownSwordProjectile_Sprite"; break;
	default: break;
	}
	float angle = atan2f(m_target->Y() - Y(), m_target->X() - X());
	ProjectileObj *shot = m_levelScreen->FireProjectile(sprite, Position(), Vector2(cosf(angle), sinf(angle)),
	                              ProjectileSpeed, (int) Damage, ProjectileScale, false, this);
	shot->EnergonType = m_currentAttackType;
	shot->ChaseTarget = true;
	shot->ChaseObj = m_target;
	shot->TurnSpeed = 1;
	shot->Orientation = angle;
	shot->Opacity(0.8f);
	shot->Flip(Flip());
}

void EnemyObj_Energon::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Energon::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS/walkAwayLS (lines 152, 156). */
		ChangeSprite("EnemyEnergonWalk_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                               : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.2f, 0.75f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 160). */
		ChangeSprite("EnemyEnergonIdle_Character");
		PlayAnimation(true);
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.2f, 0.75f);
		break;
	case ACTION_FIRE:
		/* fireProjectile (line 164): the attack animation, the shot. */
		CurrentSpeed(0);
		ChangeSprite("EnemyEnergonAttack_Character");
		PlayAnimation("Start", "BeforeAttack", false);
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	case ACTION_CHANGE_TYPE:
		/* changeAttackType (line 172): switch, then a 2 s rest. */
		CurrentSpeed(0);
		ChangeSprite("EnemyEnergonAttack_Character");
		PlayAnimation("Start", "BeforeAttack", false);
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Energon::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_FIRE:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			FireProjectile();
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 1;
		}
		else if (m_actionPhase == 1 && !IsAnimating())
		{
			ChangeSprite("EnemyEnergonIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	case ACTION_CHANGE_TYPE:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			SwitchRandomType();
			PlayAnimation("Attack", "End", false);
			m_actionPhase = 1;
			m_actionTimer = 2.0f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyEnergonIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 2;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Energon::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	UpdateAction();
}

void EnemyObj_Energon::RunBasicLogic()
{
	/* EnemyObj_Energon.cs line 240: nothing beyond the projectile
	 * radius; the melee/projectile states mix walking, firing and the
	 * type switch. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP,
	                               ACTION_FIRE, ACTION_CHANGE_TYPE };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 0, 0, 30, 60, 10 };
		PickAction(w, 5, acts);
		break;
	}
	default:
		break;
	}
}

} // namespace RogueCastle

void RogueCastle::EnemyObj_Energon::StartCooldownMove(int move)
{
	static const Action moves[3] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP };
	StartAction(moves[move]);
}
