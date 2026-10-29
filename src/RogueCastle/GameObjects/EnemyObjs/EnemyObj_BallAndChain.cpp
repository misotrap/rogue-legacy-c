/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_BallAndChain.cs. */
#include "EnemyObj_BallAndChain.h"
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

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

/* CDGMath.GetCirclePosition (CDGMath.cs line 196): degrees. */
static Vector2 GetCirclePosition(float angle, float distance, Vector2 centre)
{
	float rad = angle * PI / 180.0f;
	return Vector2(distance * cosf(rad), distance * sinf(rad)) + centre;
}

EnemyObj_BallAndChain::EnemyObj_BallAndChain(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyFlailKnight_Character", target, difficulty)
{
	Type = EnemyType::BallAndChain;
}

EnemyObj_BallAndChain::~EnemyObj_BallAndChain()
{
	/* The level screen owns the projectiles; they die with the room. */
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_BallAndChain.cs line 29), so they collapse into one. */
void EnemyObj_BallAndChain::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::BallAndChain_Miniboss_Name;
		MaxHealth = EnemyEV::BallAndChain_Miniboss_MaxHealth;
		Damage = EnemyEV::BallAndChain_Miniboss_Damage;
		XPValue = EnemyEV::BallAndChain_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::BallAndChain_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BallAndChain_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::BallAndChain_Miniboss_DropChance;
		Speed = EnemyEV::BallAndChain_Miniboss_Speed;
		TurnSpeed = EnemyEV::BallAndChain_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::BallAndChain_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::BallAndChain_Miniboss_Jump;
		CooldownTime = EnemyEV::BallAndChain_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::BallAndChain_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BallAndChain_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BallAndChain_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BallAndChain_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BallAndChain_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::BallAndChain_Miniboss_Scale.X, EnemyEV::BallAndChain_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BallAndChain_Miniboss_ProjectileScale.X, EnemyEV::BallAndChain_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BallAndChain_Miniboss_Tint.R, EnemyEV::BallAndChain_Miniboss_Tint.G, EnemyEV::BallAndChain_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::BallAndChain_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::BallAndChain_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::BallAndChain_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::BallAndChain_Miniboss_KnockBack.X, EnemyEV::BallAndChain_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::BallAndChain_Expert_Name;
		MaxHealth = EnemyEV::BallAndChain_Expert_MaxHealth;
		Damage = EnemyEV::BallAndChain_Expert_Damage;
		XPValue = EnemyEV::BallAndChain_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::BallAndChain_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BallAndChain_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::BallAndChain_Expert_DropChance;
		Speed = EnemyEV::BallAndChain_Expert_Speed;
		TurnSpeed = EnemyEV::BallAndChain_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::BallAndChain_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::BallAndChain_Expert_Jump;
		CooldownTime = EnemyEV::BallAndChain_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::BallAndChain_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BallAndChain_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BallAndChain_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BallAndChain_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BallAndChain_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::BallAndChain_Expert_Scale.X, EnemyEV::BallAndChain_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BallAndChain_Expert_ProjectileScale.X, EnemyEV::BallAndChain_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BallAndChain_Expert_Tint.R, EnemyEV::BallAndChain_Expert_Tint.G, EnemyEV::BallAndChain_Expert_Tint.B));
		MeleeRadius = EnemyEV::BallAndChain_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::BallAndChain_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::BallAndChain_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::BallAndChain_Expert_KnockBack.X, EnemyEV::BallAndChain_Expert_KnockBack.Y);
		ChainRadius = 275;   /* line 154 */
		break;
	case ADVANCED:
		Name = EnemyEV::BallAndChain_Advanced_Name;
		MaxHealth = EnemyEV::BallAndChain_Advanced_MaxHealth;
		Damage = EnemyEV::BallAndChain_Advanced_Damage;
		XPValue = EnemyEV::BallAndChain_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::BallAndChain_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BallAndChain_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::BallAndChain_Advanced_DropChance;
		Speed = EnemyEV::BallAndChain_Advanced_Speed;
		TurnSpeed = EnemyEV::BallAndChain_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::BallAndChain_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::BallAndChain_Advanced_Jump;
		CooldownTime = EnemyEV::BallAndChain_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::BallAndChain_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BallAndChain_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BallAndChain_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BallAndChain_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BallAndChain_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::BallAndChain_Advanced_Scale.X, EnemyEV::BallAndChain_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BallAndChain_Advanced_ProjectileScale.X, EnemyEV::BallAndChain_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BallAndChain_Advanced_Tint.R, EnemyEV::BallAndChain_Advanced_Tint.G, EnemyEV::BallAndChain_Advanced_Tint.B));
		MeleeRadius = EnemyEV::BallAndChain_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::BallAndChain_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::BallAndChain_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::BallAndChain_Advanced_KnockBack.X, EnemyEV::BallAndChain_Advanced_KnockBack.Y);
		ChainRadius = 350;   /* line 113 */
		ChainSpeed2Modifier = 1.5f;
		break;
	default: /* BASIC */
		Name = EnemyEV::BallAndChain_Basic_Name;
		MaxHealth = EnemyEV::BallAndChain_Basic_MaxHealth;
		Damage = EnemyEV::BallAndChain_Basic_Damage;
		XPValue = EnemyEV::BallAndChain_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::BallAndChain_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BallAndChain_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::BallAndChain_Basic_DropChance;
		Speed = EnemyEV::BallAndChain_Basic_Speed;
		TurnSpeed = EnemyEV::BallAndChain_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::BallAndChain_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::BallAndChain_Basic_Jump;
		CooldownTime = EnemyEV::BallAndChain_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::BallAndChain_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BallAndChain_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BallAndChain_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BallAndChain_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BallAndChain_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::BallAndChain_Basic_Scale.X, EnemyEV::BallAndChain_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BallAndChain_Basic_ProjectileScale.X, EnemyEV::BallAndChain_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BallAndChain_Basic_Tint.R, EnemyEV::BallAndChain_Basic_Tint.G, EnemyEV::BallAndChain_Basic_Tint.B));
		MeleeRadius = EnemyEV::BallAndChain_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::BallAndChain_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::BallAndChain_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::BallAndChain_Basic_KnockBack.X, EnemyEV::BallAndChain_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_BallAndChain::InitializeEV()
{
	m_cooldownWeights[0] = 40; m_cooldownWeights[1] = 40; m_cooldownWeights[2] = 20;
	InitializeOne(DifficultyType);
	/* The balls are created lazily in UpdateEnemy: the level screen is
	 * attached to the enemy after Initialize. */
}

void EnemyObj_BallAndChain::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_BallAndChain::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS/walkAwayLS (lines 163, 167). */
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                               : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		if (!IsAnimating())
			PlayAnimation(true);
		m_actionTimer = RandRange(1.25f, 2.75f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 171). */
		StopAnimation();
		CurrentSpeed(0);
		m_actionTimer = RandRange(1.0f, 1.5f);
		break;
	default:
		break;
	}
}

void EnemyObj_BallAndChain::UpdateEnemy(GameTime *gameTime)
{
	/* The level screen may have freed the balls (room transition):
	 * drop them before touching; the lazy creation rebuilds them. */
	if (m_ball != nullptr && !m_ball->IsAlive)
		m_ball = nullptr;
	if (m_ball2 != nullptr && !m_ball2->IsAlive)
		m_ball2 = nullptr;
	float elapsed = (float) gameTime->ElapsedGameTime;
	/* Line 200: the balls carry the damage; created on the first update
	 * (the level screen is attached after Initialize). */
	if (m_ball == nullptr && m_levelScreen != nullptr)
	{
		m_ball = new ProjectileObj("EnemyFlailKnightBall_Sprite", Position(),
		                           Vector2(1, 0), 0, (int) ProjectileDamage,
		                           ProjectileScale, false);
		m_ball->Source = this;
		m_ball->Held = true;
		m_levelScreen->AddProjectile(m_ball);
		if (DifficultyType == ADVANCED || DifficultyType == EXPERT)
		{
			m_ball2 = new ProjectileObj("EnemyFlailKnightBall_Sprite", Position(),
			                            Vector2(1, 0), 0, (int) ProjectileDamage,
			                            ProjectileScale, false);
			m_ball2->Source = this;
			m_ball2->Held = true;
			m_levelScreen->AddProjectile(m_ball2);
		}
	}
	/* Update (line 331): the chain grows to its radius, the balls orbit
	 * (X, Bounds.Top). */
	if (m_actualChainRadius < ChainRadius)
		m_actualChainRadius += elapsed * 200;
	Vector2 centre(X(), (float) Bounds().Top());
	if (m_ball != nullptr)
		m_ball->Position(GetCirclePosition(m_ballAngle, m_actualChainRadius, centre));
	if (m_ball2 != nullptr)
	{
		if (DifficultyType == ADVANCED)
			m_ball2->Position(GetCirclePosition(m_ballAngle * ChainSpeed2Modifier,
			                                   m_actualChainRadius / 2, centre));
		else
			m_ball2->Position(GetCirclePosition(-m_ballAngle * ChainSpeed2Modifier,
			                                   -m_actualChainRadius / 2, centre));
	}
	m_ballAngle += ChainSpeed * 60 * elapsed;
}

void EnemyObj_BallAndChain::RunBasicLogic()
{
	/* EnemyObj_BallAndChain.cs line 204. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	case STATE_ENGAGE:
	{
		static const int w[] = { 60, 20, 20 };
		PickAction(w, 3, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 0, 0, 100 };
		PickAction(w, 3, acts);
		break;
	}
	}
}

void EnemyObj_BallAndChain::ResetState()
{
	/* Release the balls (they are rebuilt on the next update): null
	 * the pointers first, then let the level screen free them. */
	if (m_ball != nullptr)
	{
		m_ball->IsKilled = true;
		m_ball->Visible(false);
		m_ball->IsCollidableFlag = false;
		m_ball = nullptr;
	}
	if (m_ball2 != nullptr)
	{
		m_ball2->IsKilled = true;
		m_ball2->Visible(false);
		m_ball2->IsCollidableFlag = false;
		m_ball2 = nullptr;
	}
	/* Line 463: the chain grows again from nothing. */
	m_actualChainRadius = 0;
	m_ballAngle = 0;
	EnemyObj::ResetState();
}

void EnemyObj_BallAndChain::Kill(bool giveXP)
{
	/* EnemyObj_BallAndChain.cs line 407: each ball breaks loose as a
	 * bouncing spike, flying on along its circle (slowed down by 1.5) */
	static const char *const names[] = { "BASIC", "ADVANCED", "EXPERT", "MINIBOSS" };
	const float speed = ChainSpeed * 200 / 1.5f;
	for (int i = 0; i < 2; i++)
	{
		ProjectileObj *&ball = i == 0 ? m_ball : m_ball2;
		if (ball == nullptr) continue;
		Vector2 at = ball->Position();
		ball->IsKilled = true;
		ball->Visible(false);
		ball->IsCollidableFlag = false;
		ball = nullptr;
		if (m_levelScreen == nullptr || IsHallucination()) continue;
		EnemyObj *spike = m_levelScreen->AddEnemyToCurrentRoom(EnemyType::BouncySpike, at, names[DifficultyType]);
		if (spike == nullptr) continue;
		// The tangent of the circle: the ball's angle + 90 degrees
		float angle = i == 0 ? m_ballAngle : DifficultyType == EXPERT ? -m_ballAngle * ChainSpeed2Modifier : m_ballAngle * ChainSpeed2Modifier;
		float rad = (angle + 90) * PI / 180.0f;
		spike->Heading(Vector2(cosf(rad), sinf(rad)));
		spike->Speed = i == 0 ? speed : speed * ChainSpeed2Modifier;
		spike->CurrentSpeed(spike->Speed);
		if (IsPaused) spike->PauseEnemy();
	}
	EnemyObj::Kill(giveXP);
}

} // namespace RogueCastle

void RogueCastle::EnemyObj_BallAndChain::StartCooldownMove(int move)
{
	static const Action moves[3] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP };
	StartAction(moves[move]);
}
