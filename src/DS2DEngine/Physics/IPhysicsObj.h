/* Port of DS2DEngine/src/Physics/IPhysicsObj.cs: the physics interface.
 * Weighted containers (player, enemies) and static terrain both implement
 * it, so PhysicsManager can run the same pair loop as the original.
 */
#ifndef DS2D_IPHYSICS_OBJ_H
#define DS2D_IPHYSICS_OBJ_H

#include "../GameObjs/CollisionBox.h"
#include "../Types.h"
#include <vector>

namespace DS2DEngine
{

class IPhysicsObj
{
public:
	virtual ~IPhysicsObj() = default;

	/* Physics step (PhysicsManager.cs Update). */
	virtual bool Visible() const = 0;
	virtual bool DisableAllWeight() const = 0;
	virtual bool IsWeighted() const = 0;
	virtual bool AccelerationXEnabled() const = 0;
	virtual bool AccelerationYEnabled() const = 0;
	virtual bool DisableGravity() const = 0;
	virtual float AccelerationX() const = 0;
	virtual void AccelerationX(float value) = 0;
	virtual float AccelerationY() const = 0;
	virtual void AccelerationY(float value) = 0;
	virtual void UpdatePhysics(GameTime *gameTime) = 0;
	virtual bool DisableHitboxUpdating() const = 0;
	virtual void UpdateCollisionBoxes() = 0;

	/* Pair collision pass. */
	virtual bool IsCollidable() const = 0;
	virtual int CollisionTypeTag() const = 0;
	virtual bool SameTypesCollide() const { return false; }
	virtual const std::vector<CollisionBox *> &CollisionBoxes() const = 0;
	virtual void CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType) = 0;

	/* Read by the terrain response. */
	virtual Rectangle TerrainBounds() const = 0;
	virtual bool CollidesTop() const = 0;
	virtual bool CollidesBottom() const = 0;
	virtual bool CollidesLeft() const = 0;
	virtual bool CollidesRight() const = 0;

	/* C# IDealsDamageObj: the damage this object's weapon boxes deal. */
	virtual int DealsDamage() const { return 0; }
};

} // namespace DS2DEngine

#endif
