/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_BouncySpike.cs. */
#include "EnemyObj_BouncySpike.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../TerrainObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../../../DS2DEngine/CollisionMath.h"
#include "../../../DS2DEngine/Consts.h"
#include <cmath>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static const float PI = 3.14159265358979323846f;

EnemyObj_BouncySpike::EnemyObj_BouncySpike(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyBouncySpike_Character", target, difficulty)
{
	Type = EnemyType::BouncySpike;
	NonKillable = true;
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_BouncySpike.cs line 15), so they collapse into one. */
void EnemyObj_BouncySpike::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::BouncySpike_Miniboss_Name;
		MaxHealth = EnemyEV::BouncySpike_Miniboss_MaxHealth;
		Damage = EnemyEV::BouncySpike_Miniboss_Damage;
		XPValue = EnemyEV::BouncySpike_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::BouncySpike_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BouncySpike_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::BouncySpike_Miniboss_DropChance;
		Speed = EnemyEV::BouncySpike_Miniboss_Speed;
		TurnSpeed = EnemyEV::BouncySpike_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::BouncySpike_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::BouncySpike_Miniboss_Jump;
		CooldownTime = EnemyEV::BouncySpike_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::BouncySpike_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BouncySpike_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BouncySpike_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BouncySpike_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BouncySpike_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::BouncySpike_Miniboss_Scale.X, EnemyEV::BouncySpike_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BouncySpike_Miniboss_ProjectileScale.X, EnemyEV::BouncySpike_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BouncySpike_Miniboss_Tint.R, EnemyEV::BouncySpike_Miniboss_Tint.G, EnemyEV::BouncySpike_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::BouncySpike_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::BouncySpike_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::BouncySpike_Miniboss_EngageRadius;
		break;
	case EXPERT:
		Name = EnemyEV::BouncySpike_Expert_Name;
		MaxHealth = EnemyEV::BouncySpike_Expert_MaxHealth;
		Damage = EnemyEV::BouncySpike_Expert_Damage;
		XPValue = EnemyEV::BouncySpike_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::BouncySpike_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BouncySpike_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::BouncySpike_Expert_DropChance;
		Speed = EnemyEV::BouncySpike_Expert_Speed;
		TurnSpeed = EnemyEV::BouncySpike_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::BouncySpike_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::BouncySpike_Expert_Jump;
		CooldownTime = EnemyEV::BouncySpike_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::BouncySpike_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BouncySpike_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BouncySpike_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BouncySpike_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BouncySpike_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::BouncySpike_Expert_Scale.X, EnemyEV::BouncySpike_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BouncySpike_Expert_ProjectileScale.X, EnemyEV::BouncySpike_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BouncySpike_Expert_Tint.R, EnemyEV::BouncySpike_Expert_Tint.G, EnemyEV::BouncySpike_Expert_Tint.B));
		MeleeRadius = EnemyEV::BouncySpike_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::BouncySpike_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::BouncySpike_Expert_EngageRadius;
		break;
	case ADVANCED:
		Name = EnemyEV::BouncySpike_Advanced_Name;
		MaxHealth = EnemyEV::BouncySpike_Advanced_MaxHealth;
		Damage = EnemyEV::BouncySpike_Advanced_Damage;
		XPValue = EnemyEV::BouncySpike_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::BouncySpike_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BouncySpike_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::BouncySpike_Advanced_DropChance;
		Speed = EnemyEV::BouncySpike_Advanced_Speed;
		TurnSpeed = EnemyEV::BouncySpike_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::BouncySpike_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::BouncySpike_Advanced_Jump;
		CooldownTime = EnemyEV::BouncySpike_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::BouncySpike_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BouncySpike_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BouncySpike_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BouncySpike_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BouncySpike_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::BouncySpike_Advanced_Scale.X, EnemyEV::BouncySpike_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BouncySpike_Advanced_ProjectileScale.X, EnemyEV::BouncySpike_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BouncySpike_Advanced_Tint.R, EnemyEV::BouncySpike_Advanced_Tint.G, EnemyEV::BouncySpike_Advanced_Tint.B));
		MeleeRadius = EnemyEV::BouncySpike_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::BouncySpike_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::BouncySpike_Advanced_EngageRadius;
		break;
	default: /* BASIC */
		Name = EnemyEV::BouncySpike_Basic_Name;
		MaxHealth = EnemyEV::BouncySpike_Basic_MaxHealth;
		Damage = EnemyEV::BouncySpike_Basic_Damage;
		XPValue = EnemyEV::BouncySpike_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::BouncySpike_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::BouncySpike_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::BouncySpike_Basic_DropChance;
		Speed = EnemyEV::BouncySpike_Basic_Speed;
		TurnSpeed = EnemyEV::BouncySpike_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::BouncySpike_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::BouncySpike_Basic_Jump;
		CooldownTime = EnemyEV::BouncySpike_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::BouncySpike_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::BouncySpike_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::BouncySpike_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::BouncySpike_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::BouncySpike_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::BouncySpike_Basic_Scale.X, EnemyEV::BouncySpike_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::BouncySpike_Basic_ProjectileScale.X, EnemyEV::BouncySpike_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::BouncySpike_Basic_Tint.R, EnemyEV::BouncySpike_Basic_Tint.G, EnemyEV::BouncySpike_Basic_Tint.B));
		MeleeRadius = EnemyEV::BouncySpike_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::BouncySpike_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::BouncySpike_Basic_EngageRadius;
		break;
	}
	ProjectileDamage = Damage;
	KnockBack = Vector2(1, 2);

	/* EnemyObj_BouncySpike.cs line 18: a random direction. The port keeps
	 * the orientation as a member (no Orientation property yet). */
	float orientation;
	int randomizer = rand() % 12;
	if (randomizer >= 9)
		orientation = 0;
	else if (randomizer >= 6)
		orientation = 180;
	else if (randomizer >= 4)
		orientation = 90;
	else if (randomizer >= 1)
		orientation = 270;
	else
		orientation = 45;
	m_internalOrientation = orientation;
	float rad = orientation * PI / 180.0f;
	Heading(Vector2(cosf(rad), sinf(rad)));
	CurrentSpeed(Speed);
}

void EnemyObj_BouncySpike::InitializeEV()
{
	InitializeOne(DifficultyType);
}

/* The C# heading mirror (line 223): the side normal is (mtd.Y, -mtd.X). */
void EnemyObj_BouncySpike::ReflectHeading(Vector2 mtd)
{
	Vector2 v = Heading();
	Vector2 l(mtd.Y, -mtd.X);
	float ll = l.X * l.X + l.Y * l.Y;
	if (ll == 0)
		return;
	float dot = v.X * l.X + v.Y * l.Y;
	Heading(Vector2((2.0f * (dot / ll)) * l.X - v.X,
	                (2.0f * (dot / ll)) * l.Y - v.Y));
}

void EnemyObj_BouncySpike::CountBounce()
{
	/* EnemyObj_BouncySpike.cs line 236: 12 bounces in one second. */
	m_selfDestructCounter++;
	m_selfDestructTimer = 1;
}

void EnemyObj_BouncySpike::UpdateEnemy(GameTime *gameTime)
{
	float elapsed = (float) gameTime->ElapsedGameTime;

	/* Line 208: bounce off the current room's boundary strips. */
	if (m_levelScreen != nullptr && m_levelScreen->CurrentRoom() != nullptr)
	{
		Rectangle rb = m_levelScreen->CurrentRoom()->Bounds();
		Rectangle bounds = Bounds();
		Vector2 boundaryMTD(0, 0);
		if (Y() < rb.Top() + 10)
			boundaryMTD = CollisionMath::CalculateMTD(bounds, Rectangle(rb.X, rb.Y, rb.Width, 10));
		else if (Y() > rb.Bottom() - 10)
			boundaryMTD = CollisionMath::CalculateMTD(bounds, Rectangle(rb.X, rb.Bottom() - 10, rb.Width, 10));
		if (X() > rb.Right() - 10)
			boundaryMTD = CollisionMath::CalculateMTD(bounds, Rectangle(rb.Right() - 10, rb.Y, 10, rb.Height));
		else if (X() < rb.Left() + 10)
			boundaryMTD = CollisionMath::CalculateMTD(bounds, Rectangle(rb.X, rb.Y, 10, rb.Height));
		if (boundaryMTD.X != 0 || boundaryMTD.Y != 0)
		{
			ReflectHeading(boundaryMTD);
			CountBounce();
		}
	}

	if (m_selfDestructTimer > 0)
	{
		m_selfDestructTimer -= elapsed;
		if (m_selfDestructTimer <= 0)
			m_selfDestructCounter = 0;
	}
	if (m_selfDestructCounter >= m_selfDestructTotalBounces)
		Kill(false);
	if (CurrentSpeed() == 0)
		CurrentSpeed(Speed);
	/* The visual spin (Rotation += RotationSpeed * elapsed) needs blitter
	 * rotation support. */
}

void EnemyObj_BouncySpike::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox,
                                             int collisionResponseType)
{
	/* EnemyObj_BouncySpike.cs line 253: bounce off solid, four-sided
	 * terrain. */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN)
	{
		IPhysicsObj *other = otherBox->PhysicsParent();
		if (other != nullptr && other->CollidesBottom() && other->CollidesLeft()
		    && other->CollidesRight() && other->CollidesTop())
		{
			Vector2 mtd = CollisionMath::CalculateMTD(thisBox->AbsRect(), otherBox->AbsRect());
			if (mtd.X != 0 || mtd.Y != 0)
			{
				ReflectHeading(mtd);
				X(X() + mtd.X);
				Y(Y() + mtd.Y);
				CountBounce();
				return;
			}
		}
	}
	EnemyObj::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

void EnemyObj_BouncySpike::ResetState()
{
	/* EnemyObj_BouncySpike.cs line 288 (Reset): back to the initial
	 * direction (the base resets the heading to zero). */
	EnemyObj::ResetState();
	float rad = m_internalOrientation * PI / 180.0f;
	Heading(Vector2(cosf(rad), sinf(rad)));
	CurrentSpeed(Speed);
	m_selfDestructCounter = 0;
	m_selfDestructTimer = 0;
}

} // namespace RogueCastle
