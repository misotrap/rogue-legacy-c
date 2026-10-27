/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Spark.cs. */
#include "EnemyObj_Spark.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../TerrainObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../../../DS2DEngine/CollisionMath.h"
#include <cmath>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_Spark::EnemyObj_Spark(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySpark_Character", target, difficulty)
{
	/* EnemyObj_Spark.cs line 311 (constructor). */
	IsWeightedFlag = false;
	ForceDraw(true);
	Type = EnemyType::Spark;
	NonKillable = true;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Spark.cs line 34), so they collapse into one. */
void EnemyObj_Spark::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Spark_Miniboss_Name;
		MaxHealth = EnemyEV::Spark_Miniboss_MaxHealth;
		Damage = EnemyEV::Spark_Miniboss_Damage;
		XPValue = EnemyEV::Spark_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Spark_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Spark_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Spark_Miniboss_DropChance;
		Speed = EnemyEV::Spark_Miniboss_Speed;
		TurnSpeed = EnemyEV::Spark_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Spark_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Spark_Miniboss_Jump;
		CooldownTime = EnemyEV::Spark_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Spark_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Spark_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Spark_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Spark_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Spark_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Spark_Miniboss_Scale.X, EnemyEV::Spark_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Spark_Miniboss_ProjectileScale.X, EnemyEV::Spark_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Spark_Miniboss_Tint.R, EnemyEV::Spark_Miniboss_Tint.G, EnemyEV::Spark_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Spark_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Spark_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Spark_Miniboss_EngageRadius;
		KnockBack = Vector2(EnemyEV::Spark_Miniboss_KnockBack.X, EnemyEV::Spark_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Spark_Expert_Name;
		MaxHealth = EnemyEV::Spark_Expert_MaxHealth;
		Damage = EnemyEV::Spark_Expert_Damage;
		XPValue = EnemyEV::Spark_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Spark_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Spark_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Spark_Expert_DropChance;
		Speed = EnemyEV::Spark_Expert_Speed;
		TurnSpeed = EnemyEV::Spark_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Spark_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Spark_Expert_Jump;
		CooldownTime = EnemyEV::Spark_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Spark_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Spark_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Spark_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Spark_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Spark_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Spark_Expert_Scale.X, EnemyEV::Spark_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Spark_Expert_ProjectileScale.X, EnemyEV::Spark_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Spark_Expert_Tint.R, EnemyEV::Spark_Expert_Tint.G, EnemyEV::Spark_Expert_Tint.B));
		MeleeRadius = EnemyEV::Spark_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Spark_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Spark_Expert_EngageRadius;
		KnockBack = Vector2(EnemyEV::Spark_Expert_KnockBack.X, EnemyEV::Spark_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Spark_Advanced_Name;
		MaxHealth = EnemyEV::Spark_Advanced_MaxHealth;
		Damage = EnemyEV::Spark_Advanced_Damage;
		XPValue = EnemyEV::Spark_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Spark_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Spark_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Spark_Advanced_DropChance;
		Speed = EnemyEV::Spark_Advanced_Speed;
		TurnSpeed = EnemyEV::Spark_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Spark_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Spark_Advanced_Jump;
		CooldownTime = EnemyEV::Spark_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Spark_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Spark_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Spark_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Spark_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Spark_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Spark_Advanced_Scale.X, EnemyEV::Spark_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Spark_Advanced_ProjectileScale.X, EnemyEV::Spark_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Spark_Advanced_Tint.R, EnemyEV::Spark_Advanced_Tint.G, EnemyEV::Spark_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Spark_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Spark_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Spark_Advanced_EngageRadius;
		KnockBack = Vector2(EnemyEV::Spark_Advanced_KnockBack.X, EnemyEV::Spark_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Spark_Basic_Name;
		MaxHealth = EnemyEV::Spark_Basic_MaxHealth;
		Damage = EnemyEV::Spark_Basic_Damage;
		XPValue = EnemyEV::Spark_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Spark_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Spark_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Spark_Basic_DropChance;
		Speed = EnemyEV::Spark_Basic_Speed;
		TurnSpeed = EnemyEV::Spark_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Spark_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Spark_Basic_Jump;
		CooldownTime = EnemyEV::Spark_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Spark_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Spark_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Spark_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Spark_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Spark_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Spark_Basic_Scale.X, EnemyEV::Spark_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Spark_Basic_ProjectileScale.X, EnemyEV::Spark_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Spark_Basic_Tint.R, EnemyEV::Spark_Basic_Tint.G, EnemyEV::Spark_Basic_Tint.B));
		MeleeRadius = EnemyEV::Spark_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Spark_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Spark_Basic_EngageRadius;
		KnockBack = Vector2(EnemyEV::Spark_Basic_KnockBack.X, EnemyEV::Spark_Basic_KnockBack.Y);
		break;
	}
	ProjectileDamage = Damage;
	/* InitializeLogic (line 143). */
	CurrentSpeed(Speed);
	m_orientation = 0;
	Heading(Vector2(1, 0));
}

void EnemyObj_Spark::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Spark::HookToGround()
{
	/* EnemyObj_Spark.cs line 147: snap onto the closest ground below. */
	m_hookedToGround = true;
	if (m_levelScreen == nullptr || m_levelScreen->CurrentRoom() == nullptr)
		return;
	float closestGround = 1000;
	TerrainObj *closestTerrain = nullptr;
	Rectangle probe((int) X(), (int) Y(), Width(), (int) (Height() / 2));
	for (TerrainObj *terrain : m_levelScreen->CurrentRoom()->TerrainObjList())
	{
		if (terrain->Y() >= Y() && terrain->Y() - Y() < closestGround)
		{
			Rectangle tBounds = terrain->Bounds();
			if (CollisionMath::Intersects(tBounds, probe))
			{
				closestGround = terrain->Y() - Y();
				closestTerrain = terrain;
			}
		}
	}
	if (closestTerrain != nullptr)
		Y(closestTerrain->Y() - (Height() / 2) + 5);
}

void EnemyObj_Spark::CollisionCheckRight()
{
	/* EnemyObj_Spark.cs line 227: corner and edge probes against the room
	 * bounds and the terrain steer the heading around the walls. */
	if (m_levelScreen == nullptr || m_levelScreen->CurrentRoom() == nullptr)
		return;
	Rectangle rb = m_levelScreen->CurrentRoom()->Bounds();
	Rectangle b = Bounds();
	int sz = m_collisionBoxSize;
	int w = Width(), h = Height();
	bool collidesTop = false, collidesBottom = false, collidesLeft = false, collidesRight = false;
	bool collidesTL = false, collidesTR = false, collidesBL = false, collidesBR = false;

	if (b.Right() >= rb.Right())
	{
		collidesTR = collidesRight = collidesBR = true;
	}
	else if (b.Left() <= rb.Left())
	{
		collidesTL = collidesLeft = collidesBL = true;
	}
	if (b.Top() <= rb.Top())
	{
		collidesTR = collidesTop = collidesTL = true;
	}
	else if (b.Bottom() >= rb.Bottom())
	{
		collidesBL = collidesBottom = collidesBR = true;
	}

	/* The probe rectangles (lines 354-380). Rotated hulls are skipped
	 * with the rotation support. */
	Rectangle topRect(b.X + sz, b.Y, w - 2 * sz, sz);
	Rectangle bottomRect(b.X + sz, b.Bottom() - sz, w - 2 * sz, sz);
	Rectangle leftRect(b.X, b.Y + sz, sz, h - 2 * sz);
	Rectangle rightRect(b.Right() - sz, b.Y + sz, sz, h - 2 * sz);
	Rectangle tlRect(b.X, b.Y, sz, sz);
	Rectangle trRect(b.Right() - sz, b.Y, sz, sz);
	Rectangle brRect(b.Right() - sz, b.Bottom() - sz, sz, sz);
	Rectangle blRect(b.X, b.Bottom() - sz, sz, sz);

	for (TerrainObj *terrain : m_levelScreen->CurrentRoom()->TerrainObjList())
	{
		if (terrain->Rotation() != 0)
			continue;
		Rectangle objAbs((int) terrain->X(), (int) terrain->Y(), terrain->Width(), terrain->Height());
		if (CollisionMath::Intersects(tlRect, objAbs))
			collidesTL = true;
		if (CollisionMath::Intersects(trRect, objAbs))
			collidesTR = true;
		if (CollisionMath::Intersects(brRect, objAbs))
			collidesBR = true;
		if (CollisionMath::Intersects(blRect, objAbs))
			collidesBL = true;
		if (CollisionMath::Intersects(topRect, objAbs))
			collidesTop = true;
		if (CollisionMath::Intersects(bottomRect, objAbs))
			collidesBottom = true;
		if (CollisionMath::Intersects(leftRect, objAbs))
			collidesLeft = true;
		if (CollisionMath::Intersects(rightRect, objAbs))
			collidesRight = true;
	}

	/* The corner rules (lines 294-311); the cliff cases follow (313-317).
	 * The C# Orientation is in radians: 0 = right, -90 deg = up,
	 * -180 deg = left, 90 deg = down. */
	if (collidesBR && !collidesTR && !collidesRight)
		m_orientation = 0;
	if (collidesTR && collidesBR && !collidesTL)
		m_orientation = -90.0f * PI / 180.0f;
	if (collidesTR && collidesTL && !collidesBL)
		m_orientation = -180.0f * PI / 180.0f;
	if (collidesTL && collidesLeft && !collidesBottom)
		m_orientation = 90.0f * PI / 180.0f;
	/* Special cliff cases. */
	if (collidesTR && !collidesTop && !collidesRight)
		m_orientation = -90.0f * PI / 180.0f;
	if (collidesTL && !collidesTop && !collidesLeft)
		m_orientation = -180.0f * PI / 180.0f;
	if (collidesBL && !collidesLeft && !collidesRight && !collidesBottom)
		m_orientation = 90.0f * PI / 180.0f;
	if (collidesBR && !collidesBottom && !collidesRight)
		m_orientation = 0;
	Heading(Vector2(cosf(m_orientation), sinf(m_orientation)));
}

void EnemyObj_Spark::UpdateEnemy(GameTime *gameTime)
{
	if (!m_hookedToGround)
		HookToGround();
	CollisionCheckRight();
	/* Line 217: the spark moves itself; the base Update moves flying
	 * enemies too, so it goes twice as fast - like the original. */
	Position(Position() + Heading() * (CurrentSpeed() * (float) gameTime->ElapsedGameTime));
}

void EnemyObj_Spark::ResetState()
{
	/* The C# Reset re-hooks to the ground on the next update. */
	EnemyObj::ResetState();
	m_hookedToGround = false;
	CurrentSpeed(Speed);
	m_orientation = 0;
	Heading(Vector2(1, 0));
}

} // namespace RogueCastle
