/* Port of RogueCastle/src/Game Objects/ProjectileObj.cs. */
#include "ProjectileObj.h"
#include "../../DS2DEngine/Consts.h"
#include "../../DS2DEngine/CollisionMath.h"
#include "../EVs/GameEV.h"
#include "../../debugflags.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "EnemyObjs/EnemyObj.h"
#include "../../Audio/Audio.h"
#include <cmath>

namespace RogueCastle
{

namespace ST = SpellType;

using namespace DS2DEngine;

ProjectileObj::ProjectileObj(const std::string &spriteName, Vector2 position,
                              Vector2 angle, float speed, int damage,
                              Vector2 scale, bool weighted)
	: PhysicsObjContainer("")   /* single sprite, not a character */
{
	/* The C# ProjectileObj is a PhysicsObj with the sprite set manually;
	 * the port adds one SpriteObj child to the container so the physics
	 * and drawing work through the usual paths. */
	SpriteObj *sprite = new SpriteObj(spriteName);
	AddChild(sprite);

	Position(position);
	Scale(scale);
	Damage = damage;
	m_speed = speed;
	Heading(Vector2(angle.X, angle.Y));
	CurrentSpeed(speed);
	IsWeightedFlag = weighted;
	CollisionTypeTag(3);   /* CollisionType_ENEMY */
	ForceDraw(true);

	/* A weapon box over the whole sprite (the C# ProjectileObj adds its
	 * collision box over its bounds). */
	UpdateCollisionBoxes();
	Rectangle b = RelativeBounds();   /* AddCollisionBox takes relative coords */
	AddCollisionBox(b.X, b.Y, b.Width, b.Height, Consts::WEAPON_HITBOX);
	DisableHitboxUpdatingFlag = true;
	UpdateTerrainBox();
}

bool ProjectileObj::Reflect(int playerBoxType, bool airAttacking, bool blocking)
{
	/* EnergonProjectileObj.cs line 50: the sword, the down strike or the
	 * block (by type) sends it back at the Energon, twice as fast */
	if (EnergonType < 0 || CollisionTypeTag() != 3 || Source == nullptr)
		return false;
	bool weapon = playerBoxType == Consts::WEAPON_HITBOX, body = playerBoxType == Consts::BODY_HITBOX;
	if ((EnergonType == 0 && weapon && !airAttacking) || (EnergonType == 1 && body && blocking)
	    || (EnergonType == 2 && weapon && airAttacking))
	{
		ChaseObj = (DS2DEngine::GameObj *) Source;
		CollisionTypeTag(2);
		m_speed *= 2;
		Audio::PlaySound("Player_Block");
		return true;
	}
	return false;
}

const std::string *ProjectileObj::KillerName(bool &boss) const
{
	static const std::string none;
	boss = false;
	return Source != nullptr ? Source->KillerName(boss) : &none;
}

void ProjectileObj::TurnToFace(Vector2 facePosition, float turnSpeed, float elapsed)
{
	/* ProjectileObj.cs line 262 (TurnToFace). */
	float x = facePosition.X - X(), y = facePosition.Y - Y();
	float desired = atan2f(y, x);
	float difference = desired - Orientation;
	while (difference > M_PI) difference -= 2 * (float) M_PI;
	while (difference < -M_PI) difference += 2 * (float) M_PI;
	float maxTurn = turnSpeed * 60 * elapsed;
	if (difference > maxTurn) difference = maxTurn;
	if (difference < -maxTurn) difference = -maxTurn;
	Orientation += difference;
}

void ProjectileObj::MoveByHeading(float elapsed)
{
	Position(Position() + Heading() * (m_speed * elapsed));
}

void ProjectileObj::ActivateEffect()
{
	/* ProjectileObj.cs line 562 (ActivateEffect). */
	switch (Spell)
	{
	case ST::TimeBomb:
		/* The explosion becomes a damage wall for half a second. */
		IsWeightedFlag = false;
		IsDying = true;
		CollisionTypeTag(4);   /* CollisionType_GLOBAL_DAMAGE_WALL */
		LifeSpan = 0.5f;
		m_elapsedLifeSpan = 0;
		Audio::PlaySound("Boss_Explo_01", "Boss_Explo_02", "Boss_Explo_03");
		break;
	case ST::Nuke:
		if (SourcePlayer != nullptr && SourcePlayer->LevelScreen() != nullptr)
			SourcePlayer->LevelScreen()->DamageAllEnemies(Damage);
		KillProjectile();
		break;
	case ST::Displacer:
		if (SourcePlayer != nullptr)
			SourcePlayer->Translocate(Position());
		KillProjectile();
		break;
	case ST::Boomerang:
	case ST::Bounce:
	case ST::Laser:
		/* They can damage enemies from now on (they start as player
		 * projectiles; the tag is what the enemies check). */
		CollisionTypeTag(2);
		break;
	}
}

void ProjectileObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox,
                                      int collisionResponseType)
{
	IPhysicsObj *other = otherBox->PhysicsParent();

	/* Enemy projectiles die only when they really hurt the hero: the
	 * player calls OnPlayerHit (PlayerObj.cs line 2050); his sword and
	 * spells do not destroy them */
	if ((collisionResponseType == Consts::COLLISIONRESPONSE_FIRSTBOXHIT
	     || collisionResponseType == Consts::COLLISIONRESPONSE_SECONDBOXHIT)
	    && other != nullptr && other->CollisionTypeTag() == 2)
		return;

	/* Terrain hits (ProjectileObj.cs line 294): only the spells with
	 * CollidesWithTerrain get a terrain box, so only they arrive here.
	 * The C# requires the other parent to be a TerrainObj: the hero and
	 * the enemies have terrain boxes of their own (the ground probe),
	 * and a shot must not die on the hero's one frame before its weapon
	 * box reaches his body (tag 1 = CollisionType_WALL, the port's
	 * TerrainObj; the furniture is 5, the hero 2, the enemies 3). */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN)
	{
		if (other == nullptr || other->CollisionTypeTag() != 1)
			return;
		Rectangle a = thisBox->AbsRect(), b = otherBox->AbsRect();
		Vector2 mtd = CollisionMath::CalculateMTD(a, b);
		switch (Spell)
		{
		case ST::Bounce:
		{
			/* Reflect the heading off the side it hit (the normal is
			 * the MTD axis), ProjectileObj.cs line 306. */
			Audio::PlaySound("Spike_Bounce_01", "Spike_Bounce_02", "Spike_Bounce_03");
			Vector2 normal(mtd.X != 0 ? (mtd.X > 0 ? 1.0f : -1.0f) : 0.0f,
			               mtd.Y != 0 ? (mtd.Y > 0 ? 1.0f : -1.0f) : 0.0f);
			Vector2 v(cosf(Orientation), sinf(Orientation));
			float dot = v.X * normal.X + v.Y * normal.Y;
			Vector2 reflected(v.X - 2 * dot * normal.X, v.Y - 2 * dot * normal.Y);
			Orientation = atan2f(reflected.Y, reflected.X);
			X(X() + mtd.X);
			Y(Y() + mtd.Y);
			break;
		}
		case ST::TimeBomb:
			/* The weighted bomb lands and stops (ProjectileObj.cs
			 * line 324). */
			PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
			if (mtd.Y <= 0 && mtd.X == 0)
			{
				AccelerationY(0);
				AccelerationX(0);
				IsWeightedFlag = false;
			}
			break;
		default:
			/* Everything else sticks to (or dies at) the wall. */
			if (DestroysWithTerrain)
				KillProjectile();
			else
			{
				PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
				AccelerationY(0);
				AccelerationX(0);
				IsWeightedFlag = false;
			}
			break;
		}
	}
	(void) thisBox;
}

void ProjectileObj::Update(GameTime *gameTime)
{
	/* ProjectileObj.cs line 108 (Update): the spell behaviours. */
	float elapsed = (float) gameTime->ElapsedGameTime;

	switch (Spell)
	{
	case ST::TimeBomb:
		/* Blinks faster and faster, then explodes. */
		if (BlinkTime >= AltX && AltX != 0)
			BlinkTime = AltX / 1.5f;
		if (AltX > 0)
		{
			AltX -= elapsed;
			if (AltX <= 0)
				ActivateEffect();
		}
		break;
	case ST::Nuke:
		/* Each crow flies straight briefly, then homes on its target. */
		if (AltX <= 0)
		{
			if (Target != nullptr)
				TurnToFace(Target->Position(), TurnSpeed, elapsed);
		}
		else
			AltX -= elapsed;
		Heading(Vector2(cosf(Orientation), sinf(Orientation)));
		MoveByHeading(elapsed);
		/* The crow dies with its enemy. */
		if (Target == nullptr || Target->IsKilled)
			KillProjectile();
		break;
	case ST::DamageShield:
		/* The leaves orbit their caster while the spell is active
		 * (ProjectileObj.cs line 189: the player while he casts, an
		 * enemy's fire shield - the horse, the last boss - around its
		 * Target for its whole life). */
		if ((SourcePlayer != nullptr && SourcePlayer->CastingDamageShield())
		    || (Source != nullptr && Target != nullptr))
		{
			AltX += m_speed * 60 * elapsed;
			float angle = AltX * (float) M_PI / 180.0f;
			float cx = SourcePlayer != nullptr ? SourcePlayer->X() : Target->X();
			float cy = SourcePlayer != nullptr ? SourcePlayer->Y() : Target->Y();
			Position(Vector2(cx + cosf(angle) * AltY, cy + sinf(angle) * AltY));
		}
		else
			KillProjectile();
		break;
	case ST::Bounce:
		Heading(Vector2(cosf(Orientation), sinf(Orientation)));
		MoveByHeading(elapsed);
		if (AltY > 0)
		{
			AltY -= elapsed;
			if (AltY <= 0)
				ActivateEffect();
		}
		break;
	case ST::Boomerang:
		/* Slows down, then comes back (AltX = 18 deceleration). */
		m_speed -= AltX * 60 * elapsed;
		if (AltY > 0)
		{
			AltY -= elapsed;
			if (AltY <= 0)
				ActivateEffect();
		}
		break;
	case ST::Laser:
		/* The beam flashes in, then becomes a damage wall. */
		if (AltX > 0)
		{
			AltX -= elapsed;
			if (AltX <= 0)
				ActivateEffect();
		}
		break;
	default:
		break;
	}

	/* ProjectileObj.cs line 232: homing on the hero */
	if (ChaseTarget && ChaseObj != nullptr)
	{
		TurnToFace(ChaseObj->Position(), TurnSpeed, elapsed);
		Heading(Vector2(cosf(Orientation), sinf(Orientation)));
		AccelerationX(0);
		AccelerationY(0);
		Rotation(Orientation * 180.0f / (float) M_PI);
	}
	/* Line 246: following the arc, or spinning */
	if (FollowArc && !ChaseTarget && !IsDying)
	{
		if (IsWeightedFlag)
			Rotation(atan2f(AccelerationY(), AccelerationX()) * 180.0f / (float) M_PI);
		else
			Rotation(atan2f(Heading().Y, Heading().X) * 180.0f / (float) M_PI);
	}
	else if (!ChaseTarget && RotationSpeed != 0)
		Rotation(Rotation() + RotationSpeed * 60 * elapsed);

	/* Weighted spells (axe, time bomb) fly through the physics manager
	 * (their acceleration, plus gravity); the rest move by heading. */
	if (Spell != ST::Nuke && Spell != ST::Bounce && Spell != ST::DamageShield
	    && !IsWeightedFlag)
		MoveByHeading(elapsed);

	if (LifeSpan > 0)
	{
		m_elapsedLifeSpan += elapsed;
		if (m_elapsedLifeSpan >= LifeSpan)
		{
			/* RunDestroyAnimation in the C#; the port hides it, the
			 * level screen frees it. */
			IsKilled = true;
			Visible(false);
			IsCollidableFlag = false;
		}
	}
	ObjContainer::Update(gameTime);
}

} // namespace RogueCastle
