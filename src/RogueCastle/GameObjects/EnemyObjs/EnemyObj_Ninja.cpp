/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Ninja.cs. */
#include "EnemyObj_Ninja.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../TerrainObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../../../DS2DEngine/CollisionMath.h"
#include <cmath>
#include <cstdlib>
#include "../../../Audio/Audio.h"

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

static float RandRange(float a, float b)
{
	return a + (b - a) * ((float) rand() / (float) RAND_MAX);
}

EnemyObj_Ninja::EnemyObj_Ninja(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyNinjaIdle_Character", target, difficulty)
{
	Type = EnemyType::Ninja;
	/* The C# also builds the log decoy and smoke sprites (visuals only,
	 * not ported: they are not in the loaded sheets). */
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Ninja.cs line 24), so they collapse into one. */
void EnemyObj_Ninja::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Ninja_Miniboss_Name;
		MaxHealth = EnemyEV::Ninja_Miniboss_MaxHealth;
		Damage = EnemyEV::Ninja_Miniboss_Damage;
		XPValue = EnemyEV::Ninja_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Ninja_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Ninja_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Ninja_Miniboss_DropChance;
		Speed = EnemyEV::Ninja_Miniboss_Speed;
		TurnSpeed = EnemyEV::Ninja_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Ninja_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Ninja_Miniboss_Jump;
		CooldownTime = EnemyEV::Ninja_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Ninja_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Ninja_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Ninja_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Ninja_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Ninja_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Ninja_Miniboss_Scale.X, EnemyEV::Ninja_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Ninja_Miniboss_ProjectileScale.X, EnemyEV::Ninja_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Ninja_Miniboss_Tint.R, EnemyEV::Ninja_Miniboss_Tint.G, EnemyEV::Ninja_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Ninja_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Ninja_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Ninja_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Ninja_Miniboss_KnockBack.X, EnemyEV::Ninja_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Ninja_Expert_Name;
		MaxHealth = EnemyEV::Ninja_Expert_MaxHealth;
		Damage = EnemyEV::Ninja_Expert_Damage;
		XPValue = EnemyEV::Ninja_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Ninja_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Ninja_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Ninja_Expert_DropChance;
		Speed = EnemyEV::Ninja_Expert_Speed;
		TurnSpeed = EnemyEV::Ninja_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Ninja_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Ninja_Expert_Jump;
		CooldownTime = EnemyEV::Ninja_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Ninja_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Ninja_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Ninja_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Ninja_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Ninja_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Ninja_Expert_Scale.X, EnemyEV::Ninja_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Ninja_Expert_ProjectileScale.X, EnemyEV::Ninja_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Ninja_Expert_Tint.R, EnemyEV::Ninja_Expert_Tint.G, EnemyEV::Ninja_Expert_Tint.B));
		MeleeRadius = EnemyEV::Ninja_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Ninja_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Ninja_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Ninja_Expert_KnockBack.X, EnemyEV::Ninja_Expert_KnockBack.Y);
		ChanceToTeleport = 0.65f;   /* line 112 */
		break;
	case ADVANCED:
		Name = EnemyEV::Ninja_Advanced_Name;
		MaxHealth = EnemyEV::Ninja_Advanced_MaxHealth;
		Damage = EnemyEV::Ninja_Advanced_Damage;
		XPValue = EnemyEV::Ninja_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Ninja_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Ninja_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Ninja_Advanced_DropChance;
		Speed = EnemyEV::Ninja_Advanced_Speed;
		TurnSpeed = EnemyEV::Ninja_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Ninja_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Ninja_Advanced_Jump;
		CooldownTime = EnemyEV::Ninja_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Ninja_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Ninja_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Ninja_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Ninja_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Ninja_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Ninja_Advanced_Scale.X, EnemyEV::Ninja_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Ninja_Advanced_ProjectileScale.X, EnemyEV::Ninja_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Ninja_Advanced_Tint.R, EnemyEV::Ninja_Advanced_Tint.G, EnemyEV::Ninja_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Ninja_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Ninja_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Ninja_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Ninja_Advanced_KnockBack.X, EnemyEV::Ninja_Advanced_KnockBack.Y);
		ChanceToTeleport = 0.50f;   /* line 151 */
		break;
	default: /* BASIC */
		Name = EnemyEV::Ninja_Basic_Name;
		MaxHealth = EnemyEV::Ninja_Basic_MaxHealth;
		Damage = EnemyEV::Ninja_Basic_Damage;
		XPValue = EnemyEV::Ninja_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Ninja_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Ninja_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Ninja_Basic_DropChance;
		Speed = EnemyEV::Ninja_Basic_Speed;
		TurnSpeed = EnemyEV::Ninja_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Ninja_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Ninja_Basic_Jump;
		CooldownTime = EnemyEV::Ninja_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Ninja_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Ninja_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Ninja_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Ninja_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Ninja_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Ninja_Basic_Scale.X, EnemyEV::Ninja_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Ninja_Basic_ProjectileScale.X, EnemyEV::Ninja_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Ninja_Basic_Tint.R, EnemyEV::Ninja_Basic_Tint.G, EnemyEV::Ninja_Basic_Tint.B));
		MeleeRadius = EnemyEV::Ninja_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Ninja_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Ninja_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Ninja_Basic_KnockBack.X, EnemyEV::Ninja_Basic_KnockBack.Y);
		break;
	}
}

void EnemyObj_Ninja::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Ninja::ThrowShuriken(float angleOffsetDeg)
{
	/* ProjectileManager.cs line 75: aim at the target plus the offset,
	 * from the SourceAnchor (15, 0). */
	float angle = atan2f(m_target->Y() - Y(), m_target->X() - X()) * 180.0f / PI;
	if (Flip() == SpriteEffects::FlipHorizontally)
		angle -= angleOffsetDeg;
	else
		angle += angleOffsetDeg;
	float rad = angle * PI / 180.0f;
	m_levelScreen->FireProjectile("ShurikenProjectile1_Sprite",
	                              Position() + Vector2(15, 0), Vector2(cosf(rad), sinf(rad)),
	                              ProjectileSpeed, (int) ProjectileDamage,
	                              ProjectileScale, false, this, 0, true);   /* CollidesWithTerrain (the C# ProjectileData) */
}

TerrainObj *EnemyObj_Ninja::FindClosestCeiling()
{
	/* EnemyObj_Ninja.cs line 397: the closest terrain above with
	 * CollidesBottom. */
	if (m_levelScreen == nullptr || m_levelScreen->CurrentRoom() == nullptr)
		return nullptr;
	int closestDistance = 100000000;
	TerrainObj *closestObj = nullptr;
	Rectangle test(Bounds().X, Bounds().Y - 2000, Bounds().Width, Bounds().Height + 2000);
	for (TerrainObj *terrain : m_levelScreen->CurrentRoom()->TerrainObjList())
	{
		if (!terrain->CollidesBottom() || terrain->Rotation() != 0)
			continue;
		if (!CollisionMath::Intersects(terrain->Bounds(), test))
			continue;
		int distance = 100000000;
		if (terrain->TerrainBounds().Bottom() < TerrainBounds().Top())
			distance = TerrainBounds().Top() - terrain->Bounds().Bottom();
		if (distance < closestDistance)
		{
			closestDistance = distance;
			closestObj = terrain;
		}
	}
	return closestObj;
}

void EnemyObj_Ninja::PickAction(const int *weights, int count, const Action *actions)
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

void EnemyObj_Ninja::StartAction(Action action)
{
	m_action = action;
	m_actionActive = true;
	m_actionPhase = 0;
	m_endPhase = 0;

	switch (action)
	{
	case ACTION_WALK_TOWARDS:
	case ACTION_WALK_AWAY:
		/* walkTowardsLS/walkAwayLS (lines 155, 160). */
		ChangeSprite("EnemyNinjaRun_Character");
		PlayAnimation(true);
		HeadingX(m_target->X() >= X() ? (action == ACTION_WALK_TOWARDS ? 1 : -1)
		                               : (action == ACTION_WALK_TOWARDS ? -1 : 1));
		CurrentSpeed(Speed);
		m_actionTimer = RandRange(0.25f, 0.85f);
		break;
	case ACTION_WALK_STOP:
		/* walkStopLS (line 165): idle, animation stopped. */
		ChangeSprite("EnemyNinjaIdle_Character");
		StopAnimation();
		CurrentSpeed(0);
		m_actionTimer = RandRange(0.25f, 0.85f);
		break;
	case ACTION_THROW:
		/* basicThrowShurikenLS (line 183): a pause, the throw frames,
		 * the shuriken, the recovery. */
		CurrentSpeed(0);
		ChangeSprite("EnemyNinjaIdle_Character");
		PlayAnimation(true);
		m_actionTimer = PauseBeforeProjectile;
		m_runCooldown = true;
		m_cooldownTimer = CooldownTime;
		m_endPhase = 2;
		break;
	default:
		break;
	}
}

void EnemyObj_Ninja::UpdateAction()
{
	switch (m_action)
	{
	case ACTION_THROW:
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			/* The fan per difficulty (lines 192, 203, 214). */
			Audio::PlaySound("Ninja_ThrowStar_01", "Ninja_ThrowStar_02", "Ninja_ThrowStar_03");   /* EnemyObj_Ninja.cs line 196 */
			ChangeSprite("EnemyNinjaThrow_Character");
			PlayAnimation(1, 3, false);
			switch (DifficultyType)
			{
			case EXPERT:
				ThrowShuriken(0); ThrowShuriken(-5); ThrowShuriken(5);
				ThrowShuriken(-25); ThrowShuriken(25);
				break;
			case ADVANCED:
				ThrowShuriken(0); ThrowShuriken(-10); ThrowShuriken(10);
				break;
			default:
				ThrowShuriken(0);
				break;
			}
			PlayAnimation(4, 5, false);
			m_actionPhase = 1;
			m_actionTimer = PauseAfterProjectile;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0 && !IsAnimating())
			m_actionPhase = 2;
		break;
	case ACTION_TELEPORT:
		/* teleportAttackLS (line 275): gone, then above the player. */
		if (m_actionPhase == 0 && m_actionTimer <= 0)
		{
			if (m_closestCeiling != nullptr)
			{
				/* CreateSmoke (line 511): reappear on the ceiling, at
				 * the player's x. */
				UpdateCollisionBoxes();
				Y(m_closestCeiling->TerrainBounds().Bottom() + (Y() - TerrainBounds().Top()));
				X(m_target->X());
				ChangeSprite("EnemyNinjaAttack_Character");
				PlayAnimation(true);
				Visible(true);
				Acceleration(Vector2(0, 0));
				CurrentSpeed(0);
			}
			else
				Visible(true);
			m_actionPhase = 1;
			m_actionTimer = 0.15f;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			IsWeightedFlag = true;
			IsCollidableFlag = true;
			m_actionPhase = 2;
			m_actionTimer = 0.15f;
		}
		else if (m_actionPhase == 2 && m_actionTimer <= 0)
		{
			ChangeSprite("EnemyNinjaIdle_Character");
			PlayAnimation(true);
			m_actionPhase = 3;
		}
		break;
	default:
		break;
	}
}

void EnemyObj_Ninja::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* Line 352: the ninja never leaves through the room's top. */
	if (m_levelScreen != nullptr && m_levelScreen->CurrentRoom() != nullptr
	    && Y() < m_levelScreen->CurrentRoom()->Y())
		Y(m_levelScreen->CurrentRoom()->Y());
	UpdateAction();
}

void EnemyObj_Ninja::RunBasicLogic()
{
	/* EnemyObj_Ninja.cs line 283.
	 * walkTowardsLS, walkAwayLS, walkStopLS, throwShurikenLS. */
	static const Action acts[] = { ACTION_WALK_TOWARDS, ACTION_WALK_AWAY, ACTION_WALK_STOP, ACTION_THROW };
	switch (State)
	{
	case STATE_MELEE_ENGAGE:
	case STATE_PROJECTILE_ENGAGE:
	{
		static const int w[] = { 40, 30, 0, 30 };
		PickAction(w, 4, acts);
		break;
	}
	case STATE_ENGAGE:
	{
		static const int w[] = { 65, 35, 0, 0 };
		PickAction(w, 4, acts);
		break;
	}
	default: /* WANDER */
	{
		static const int w[] = { 50, 50, 0, 0 };
		PickAction(w, 4, acts);
		break;
	}
	}
}

void EnemyObj_Ninja::RunAdvancedLogic()
{
	RunBasicLogic();
}

void EnemyObj_Ninja::RunExpertLogic()
{
	RunBasicLogic();
}

void EnemyObj_Ninja::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	/* EnemyObj_Ninja.cs line 363: a chance to teleport to the ceiling
	 * instead of taking the hit. */
	/* In the shipped game the teleport never happens: it needs
	 * m_closestCeiling, which is only set inside that very branch
	 * (EnemyObj_Ninja.cs line 439). Kept off to match. */
	if (false && m_target != nullptr && m_target->CurrentHealth > 0 && m_action != ACTION_TELEPORT
	    && (float) rand() / (float) RAND_MAX <= ChanceToTeleport)
	{
		TerrainObj *ceiling = FindClosestCeiling();
		if (ceiling != nullptr)
		{
			int distance = TerrainBounds().Top() - ceiling->TerrainBounds().Bottom();
			if (distance > 150 && distance < 700)
			{
				m_closestCeiling = ceiling;
				/* CreateLog (line 491): gone (the log decoy and the
				 * smoke are not ported). */
				Audio::PlaySound("Ninja_Teleport");   /* EnemyObj_Ninja.cs line 503 */
				Visible(false);
				IsCollidableFlag = false;
				IsWeightedFlag = false;
				m_action = ACTION_TELEPORT;
				m_actionActive = true;
				m_actionPhase = 0;
				m_actionTimer = TeleportDelay;
				m_endPhase = 3;
			}
		}
	}
	EnemyObj::HitEnemy(damage, collisionPt, isPlayer);
}

void EnemyObj_Ninja::ResetState()
{
	/* The teleport never survives a room transition. */
	if (m_action == ACTION_TELEPORT)
	{
		Visible(true);
		IsCollidableFlag = true;
		IsWeightedFlag = true;
	}
	m_closestCeiling = nullptr;
	m_action = ACTION_NONE;
	EnemyObj::ResetState();
}

} // namespace RogueCastle
