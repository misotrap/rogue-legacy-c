/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Eyeball.cs. */
#include "EnemyObj_Eyeball.h"
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

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Eyeball::EnemyObj_Eyeball(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyEyeballIdle_Character", target, difficulty)
{
	/* EnemyObj_Eyeball.cs line 690: the pupil tracks the player. */
	m_pupil = new SpriteObj("EnemyEyeballPupil_Sprite");
	AddChild(m_pupil);
	Type = EnemyType::Eyeball;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Eyeball.cs line 37), so they collapse into one. */
void EnemyObj_Eyeball::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Eyeball_Miniboss_Name;
		MaxHealth = EnemyEV::Eyeball_Miniboss_MaxHealth;
		Damage = EnemyEV::Eyeball_Miniboss_Damage;
		XPValue = EnemyEV::Eyeball_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Eyeball_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Eyeball_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Eyeball_Miniboss_DropChance;
		Speed = EnemyEV::Eyeball_Miniboss_Speed;
		TurnSpeed = EnemyEV::Eyeball_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Eyeball_Miniboss_ProjectileSpeed;
		CooldownTime = EnemyEV::Eyeball_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Eyeball_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Eyeball_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Eyeball_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Eyeball_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Eyeball_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Eyeball_Miniboss_Scale.X, EnemyEV::Eyeball_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Eyeball_Miniboss_ProjectileScale.X, EnemyEV::Eyeball_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Eyeball_Miniboss_Tint.R, EnemyEV::Eyeball_Miniboss_Tint.G, EnemyEV::Eyeball_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Eyeball_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Eyeball_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Eyeball_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Eyeball_Miniboss_KnockBack.X, EnemyEV::Eyeball_Miniboss_KnockBack.Y);
		PupilOffset(0);   /* EnemyObj_Eyeball.cs line 202 */
		m_pupilOffsetTarget = 0;
		m_resetSpriteName = "EnemyEyeballBossEye_Character";   /* line 204 */
		break;
	case EXPERT:
		Name = EnemyEV::Eyeball_Expert_Name;
		MaxHealth = EnemyEV::Eyeball_Expert_MaxHealth;
		Damage = EnemyEV::Eyeball_Expert_Damage;
		XPValue = EnemyEV::Eyeball_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Eyeball_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Eyeball_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Eyeball_Expert_DropChance;
		Speed = EnemyEV::Eyeball_Expert_Speed;
		TurnSpeed = EnemyEV::Eyeball_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Eyeball_Expert_ProjectileSpeed;
		CooldownTime = EnemyEV::Eyeball_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Eyeball_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Eyeball_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Eyeball_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Eyeball_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Eyeball_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Eyeball_Expert_Scale.X, EnemyEV::Eyeball_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Eyeball_Expert_ProjectileScale.X, EnemyEV::Eyeball_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Eyeball_Expert_Tint.R, EnemyEV::Eyeball_Expert_Tint.G, EnemyEV::Eyeball_Expert_Tint.B));
		MeleeRadius = EnemyEV::Eyeball_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Eyeball_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Eyeball_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Eyeball_Expert_KnockBack.X, EnemyEV::Eyeball_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Eyeball_Advanced_Name;
		MaxHealth = EnemyEV::Eyeball_Advanced_MaxHealth;
		Damage = EnemyEV::Eyeball_Advanced_Damage;
		XPValue = EnemyEV::Eyeball_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Eyeball_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Eyeball_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Eyeball_Advanced_DropChance;
		Speed = EnemyEV::Eyeball_Advanced_Speed;
		TurnSpeed = EnemyEV::Eyeball_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Eyeball_Advanced_ProjectileSpeed;
		CooldownTime = EnemyEV::Eyeball_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Eyeball_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Eyeball_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Eyeball_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Eyeball_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Eyeball_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Eyeball_Advanced_Scale.X, EnemyEV::Eyeball_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Eyeball_Advanced_ProjectileScale.X, EnemyEV::Eyeball_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Eyeball_Advanced_Tint.R, EnemyEV::Eyeball_Advanced_Tint.G, EnemyEV::Eyeball_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Eyeball_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Eyeball_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Eyeball_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Eyeball_Advanced_KnockBack.X, EnemyEV::Eyeball_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Eyeball_Basic_Name;
		MaxHealth = EnemyEV::Eyeball_Basic_MaxHealth;
		Damage = EnemyEV::Eyeball_Basic_Damage;
		XPValue = EnemyEV::Eyeball_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Eyeball_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Eyeball_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Eyeball_Basic_DropChance;
		Speed = EnemyEV::Eyeball_Basic_Speed;
		TurnSpeed = EnemyEV::Eyeball_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Eyeball_Basic_ProjectileSpeed;
		CooldownTime = EnemyEV::Eyeball_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Eyeball_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Eyeball_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Eyeball_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Eyeball_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Eyeball_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Eyeball_Basic_Scale.X, EnemyEV::Eyeball_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Eyeball_Basic_ProjectileScale.X, EnemyEV::Eyeball_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Eyeball_Basic_Tint.R, EnemyEV::Eyeball_Basic_Tint.G, EnemyEV::Eyeball_Basic_Tint.B));
		MeleeRadius = EnemyEV::Eyeball_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Eyeball_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Eyeball_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Eyeball_Basic_KnockBack.X, EnemyEV::Eyeball_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Eyeball::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Eyeball::FireAtAngle(float angleDeg)
{
	float rad = angleDeg * PI / 180.0f;
	Vector2 heading(cosf(rad), sinf(rad));
	m_levelScreen->FireProjectile("EyeballProjectile_Sprite", Position(), heading,
	                              ProjectileSpeed, (int) ProjectileDamage,
	                              ProjectileScale, false, this);
}

void EnemyObj_Eyeball::FireAtTarget(float angleOffsetDeg)
{
	/* ProjectileManager.cs line 75: aim at the target, plus the angle
	 * offset (subtracted when the source faces left). */
	float angle = atan2f(m_target->Y() - Y(), m_target->X() - X()) * 180.0f / PI;
	if (Flip() == SpriteEffects::FlipHorizontally)
		angle -= angleOffsetDeg;
	else
		angle += angleOffsetDeg;
	FireAtAngle(angle);
}

/* ThrowThreeProjectiles (EnemyObj_Eyeball.cs line 368): a 0/+45/-45 fan. */
void EnemyObj_Eyeball::ThrowFan(float angleOffsetDeg)
{
	FireAtTarget(angleOffsetDeg);
	FireAtTarget(angleOffsetDeg + 45.0f);
	FireAtTarget(angleOffsetDeg - 45.0f);
}

/* ThrowSprayProjectiles (line 525): 13 shots, 30 degrees apart. */
void EnemyObj_Eyeball::ThrowSprayVolley(int startDeg)
{
	for (int i = 0; i <= 360; i += 30)
		FireAtAngle(startDeg + i);
}

/* ThrowRandomProjectiles (line 580): one random angle per 45-degree sector. */
void EnemyObj_Eyeball::ThrowRandomShots()
{
	for (int sector = 0; sector < 360; sector += 45)
		FireAtAngle(RandRange((float) sector, (float) sector + 44.0f));
}

void EnemyObj_Eyeball::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Eyeball::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_shotsLeft = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_NOTHING:
		/* doNothing (line 339): just a short pause. */
		m_actionTimer = RandRange(0.2f, 0.5f);
		break;
	case ACTION_FIRE:
		/* fireProjectileBasicLS/AdvancedLS/ExpertLS (lines 224, 234, 251):
		 * windup with the fire sprite, then the shots, then idle. */
		ChangeSprite(DifficultyType == MINIBOSS ? "EnemyEyeballBossFire_Character"
		                                       : "EnemyEyeballFire_Character");
		PlayAnimation(true);
		m_actionTimer = FireballDelay;
		if (DifficultyType == ADVANCED)
			m_shotsLeft = 4;          /* four shots, 0.15 s apart */
		else if (DifficultyType == EXPERT)
			m_shotsLeft = 4;          /* four three-shot fans, 0.1 s apart */
		else
			m_shotsLeft = 1;
		m_fireInterval = DifficultyType == EXPERT ? 0.1f : 0.15f;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 1;   /* phase 1 is the idle delay after the shots */
		break;
	case ACTION_CARDINAL_SPIN:
		/* fireProjectileCardinalSpinLS (line 263). */
		ChangeSprite("EnemyEyeballBossFire_Character");
		PlayAnimation(true);
		m_pupilOffsetTarget = 0;   /* LockEyeball */
		m_pupil->ChangeSprite("EnemyEyeballBossPupilFire_Sprite");
		m_actionTimer = FireballDelay;
		m_flipper = (rand() % 2 == 0) ? -1 : 1;   /* CDGMath.RandomPlusMinus */
		m_fanIndex = 0;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	case ACTION_SPRAY:
		/* fireProjectileSprayLS (line 299): volleys every 0.8 s. */
		ChangeSprite("EnemyEyeballBossFire_Character");
		PlayAnimation(true);
		m_pupilOffsetTarget = 0;
		m_pupil->ChangeSprite("EnemyEyeballBossPupilFire_Sprite");
		m_actionTimer = FireballDelay;
		m_sprayVolley = 0;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	case ACTION_RANDOM:
		/* fireProjectileRandomLS (line 316): five random shots. */
		ChangeSprite("EnemyEyeballBossFire_Character");
		PlayAnimation(true);
		m_pupilOffsetTarget = 0;
		m_pupil->ChangeSprite("EnemyEyeballBossPupilFire_Sprite");
		m_actionTimer = FireballDelay;
		m_randomShots = 5;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Eyeball::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_FIRE:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* Fire the volley, wait between shots (line 239/386). */
			if (DifficultyType == EXPERT)
				ThrowFan(0);
			else
				FireAtTarget(0);
			m_shotsLeft--;
			if (m_shotsLeft > 0)
				m_actionTimer = m_fireInterval;
			else
			{
				ChangeSprite(DifficultyType == MINIBOSS ? "EnemyEyeballBossEye_Character"
				                                       : "EnemyEyeballIdle_Character");
				PlayAnimation(true);
				m_actionPhase = 1;
				/* Line 230/248/258: the idle delay after the attack. */
				if (DifficultyType == ADVANCED)
					m_actionTimer = RandRange(0.75f, 2.0f);
				else
					m_actionTimer = RandRange(1.0f, 3.0f);
			}
		}
		break;
	case ACTION_CARDINAL_SPIN:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			m_actionPhase = 1;
			m_actionTimer = 0.1f;   /* line 269 */
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			/* ThrowCardinalProjectiles (line 429): four shots per step,
			 * 10 degrees further each time, 17 steps (line 271). */
			float step = m_fanIndex * 10.0f * m_flipper;
			FireAtAngle(-10.0f + step);
			FireAtAngle(80.0f + step);
			FireAtAngle(170.0f + step);
			FireAtAngle(260.0f + step);
			m_fanIndex++;
			if (m_fanIndex >= 17)
			{
				m_actionPhase = 2;
				m_actionTimer = 3.15f;   /* line 272 */
			}
			else
				m_actionTimer = 0.12f;
		}
		else if (m_actionPhase == 2 && m_actionTimer <= 0)
		{
			/* Back to the boss eye (line 273). */
			ChangeSprite("EnemyEyeballBossEye_Character");
			PlayAnimation(true);
			m_pupil->ChangeSprite("EnemyEyeballBossPupil_Sprite");
			m_pupilOffsetTarget = 30;   /* UnlockEyeball */
			m_actionPhase = 3;
		}
		break;
	case ACTION_SPRAY:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			m_actionPhase = 1;
			m_actionTimer = 0.1f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			/* Volleys at 0, 0.8, 1.6 and 2.4 s (line 307 + the tween in
			 * ThrowSprayProjectiles). */
			ThrowSprayVolley(m_sprayVolley % 2 == 0 ? 10 : 20);
			m_sprayVolley++;
			if (m_sprayVolley >= 4)
			{
				ChangeSprite("EnemyEyeballBossEye_Character");
				PlayAnimation(true);
				m_pupil->ChangeSprite("EnemyEyeballBossPupil_Sprite");
				m_pupilOffsetTarget = 30;
				m_actionPhase = 2;
			}
			else
				m_actionTimer = 0.8f;
		}
		break;
	case ACTION_RANDOM:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			m_actionPhase = 1;
			m_actionTimer = 0.1f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			/* Five random shots, 0.575 s apart (line 325). */
			ThrowRandomShots();
			m_randomShots--;
			if (m_randomShots <= 0)
			{
				ChangeSprite("EnemyEyeballBossEye_Character");
				PlayAnimation(true);
				m_pupil->ChangeSprite("EnemyEyeballBossPupil_Sprite");
				m_pupilOffsetTarget = 30;
				m_actionPhase = 2;
			}
			else
				m_actionTimer = 0.575f;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Eyeball::UpdateEnemy(GameTime *gameTime)
{
	/* EnemyObj_Eyeball.cs line 700: the pupil looks at the player. */
	float angle = atan2f(m_target->Y() - Y(), m_target->X() - X());
	m_pupil->X(cosf(angle) * m_pupilOffset);
	m_pupil->Y(sinf(angle) * m_pupilOffset);

	/* LockEyeball/UnlockEyeball tween PupilOffset over 0.5 s. */
	float elapsed = (float) gameTime->ElapsedGameTime;
	float step = 30.0f / 0.5f * elapsed;   /* 30 units in 0.5 s */
	float current = (float) m_pupilOffset;
	float target = (float) m_pupilOffsetTarget;
	if (current < target)
		m_pupilOffset = (int) (current + (step < target - current ? step : target - current));
	else if (current > target)
		m_pupilOffset = (int) (current - (step < current - target ? step : current - target));
	UpdateAction();
}

void EnemyObj_Eyeball::RunBasicLogic()
{
	/* EnemyObj_Eyeball.cs line 620: fire when engaged, pause when
	 * wandering (m_generalBasicLB, 100/0). */
	static const Action acts[] = { ACTION_FIRE, ACTION_NOTHING };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 100, 0 };
		PickAction(w, 2, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 0, 100 };
		PickAction(w, 2, acts);
		break;
	}
	}
}

void EnemyObj_Eyeball::RunAdvancedLogic()
{
	/* Line 637: same sets, the Advanced difficulty fires four shots. */
	RunBasicLogic();
}

void EnemyObj_Eyeball::RunExpertLogic()
{
	/* Line 654: same sets, the Expert difficulty fires four fans. */
	RunBasicLogic();
}

void EnemyObj_Eyeball::RunMinibossLogic()
{
	/* Line 671: Khidr alternates the cardinal spin, the spray and random
	 * shots (40, 20, 40, 0). */
	static const Action acts[] = { ACTION_CARDINAL_SPIN, ACTION_SPRAY, ACTION_RANDOM, ACTION_NOTHING };
	static const int w[] = { 40, 20, 40, 0 };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	case STATE_WANDER:
		PickAction(w, 4, acts);
		break;
	default:
		break;
	}
}

} // namespace RogueCastle
