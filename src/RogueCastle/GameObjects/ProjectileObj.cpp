/* Port of RogueCastle/src/Game Objects/ProjectileObj.cs (subset). */
#include "ProjectileObj.h"
#include "../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

ProjectileObj::ProjectileObj(const std::string &spriteName, Vector2 position,
                              Vector2 angle, float speed, int damage,
                              Vector2 scale, bool weighted)
	: PhysicsObjContainer("")   /* the bone is a single sprite, not a character */
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
	Rectangle b = Bounds();
	AddCollisionBox(b.X, b.Y, b.Width, b.Height, Consts::WEAPON_HITBOX);
	DisableHitboxUpdatingFlag = true;
	UpdateTerrainBox();
}

void ProjectileObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox,
                                      int collisionResponseType)
{
	/* The bone dies when it touches the player (RunDestroyAnimation in the
	 * C#); terrain hits are not enabled for bones. */
	DS2DEngine::IPhysicsObj *other = otherBox->PhysicsParent();
	if ((collisionResponseType == Consts::COLLISIONRESPONSE_FIRSTBOXHIT
	     || collisionResponseType == Consts::COLLISIONRESPONSE_SECONDBOXHIT)
	    && other != nullptr && other->CollisionTypeTag() == 2)
	{
		IsKilled = true;
		Visible(false);
		IsCollidableFlag = false;
	}
	(void) thisBox;
}

void ProjectileObj::Update(GameTime *gameTime)
{
	/* ProjectileObj.cs line 158: Position += Heading * (CurrentSpeed *
	 * elapsed); gravity comes from the physics manager like the original. */
	float elapsed = (float) gameTime->ElapsedGameTime;
	Position(Position() + Heading() * (m_speed * elapsed));
	ObjContainer::Update(gameTime);
}

} // namespace RogueCastle
