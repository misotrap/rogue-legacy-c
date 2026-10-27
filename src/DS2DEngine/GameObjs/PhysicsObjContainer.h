/* Port of DS2DEngine/src/GameObjs/PhysicsObjContainer.cs: a container whose
 * children are PhysicsObjs; its terrain bounds are the union of theirs and
 * its collision boxes are shared with the children (added by them).
 * Implements the physics interface like the C# class.
 */
#ifndef DS2D_PHYSICS_OBJ_CONTAINER_H
#define DS2D_PHYSICS_OBJ_CONTAINER_H

#include "ObjContainer.h"
#include "../Physics/IPhysicsObj.h"
#include "PhysicsObj.h"
#include <vector>

namespace DS2DEngine
{

class PhysicsObjContainer : public ObjContainer, public IPhysicsObj
{
public:
	PhysicsObjContainer();
	explicit PhysicsObjContainer(const std::string &spriteName);

	PhysicsObjContainer *AsPhysicsObjContainer() override { return this; }

	void UpdateCollisionBoxes() override;
	void UpdatePhysics(GameTime *gameTime) override;
	void CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType) override;

	/* --- IPhysicsObj --- */
	bool Visible() const override { return ObjContainer::Visible(); }
	bool DisableAllWeight() const override { return DisableAllWeightFlag; }
	bool IsWeighted() const override { return IsWeightedFlag; }
	bool AccelerationXEnabled() const override { return AccelerationXFlag; }
	bool AccelerationYEnabled() const override { return AccelerationYFlag; }
	bool DisableGravity() const override { return DisableGravityFlag; }
	float AccelerationX() const override { return m_acceleration.X; }
	void AccelerationX(float value) override { m_acceleration.X = value; }
	float AccelerationY() const override { return m_acceleration.Y; }
	void AccelerationY(float value) override { m_acceleration.Y = value; }
	bool DisableHitboxUpdating() const override { return DisableHitboxUpdatingFlag; }
	bool IsCollidable() const override { return IsCollidableFlag; }
	int CollisionTypeTag() const override { return m_collisionTypeTag; }
	void CollisionTypeTag(int value) { m_collisionTypeTag = value; }
	bool SameTypesCollide() const override { return SameTypesCollideFlag; }
	bool SameTypesCollideFlag = false;
	const std::vector<CollisionBox *> &CollisionBoxes() const override { return m_collisionBoxes; }
	Rectangle TerrainBounds() const override { return m_terrainBounds; }
	bool CollidesTop() const override { return CollidesTopFlag; }
	bool CollidesBottom() const override { return CollidesBottomFlag; }
	bool CollidesLeft() const override { return CollidesLeftFlag; }
	bool CollidesRight() const override { return CollidesRightFlag; }

	/* C# public fields, kept as Flag-suffixed members because the IPhysicsObj
	 * getters use the same names. */
	bool IsWeightedFlag = true;
	bool IsCollidableFlag = true;
	bool CollidesTopFlag = true;
	bool CollidesBottomFlag = true;
	bool CollidesLeftFlag = true;
	bool CollidesRightFlag = true;
	bool AccelerationXFlag = true;
	bool AccelerationYFlag = true;
	bool DisableGravityFlag = false;
	bool DisableAllWeightFlag = false;
	bool DisableHitboxUpdatingFlag = false;
	int m_collisionTypeTag = 0;

	/* Called by the children when they create shared boxes. */
	void AddSharedCollisionBox(CollisionBox *box) { m_collisionBoxes.push_back(box); }

	/* Manually added collision box (C# AddCollisionBox), relative to the
	 * origin like the others. */
	void AddCollisionBox(int xPos, int yPos, int width, int height, int hitboxType);

	/* BreakableObj.cs UpdateTerrainBox: recompute the terrain bounds from
	 * the container's own collision boxes (needed when they are added
	 * manually, since UpdateCollisionBoxes only unions the children). */
	void UpdateTerrainBox();

protected:
	void BuildFromCharData(const std::string &spriteName) override;

	std::vector<CollisionBox *> m_collisionBoxes;   /* shared with children */
	Vector2 m_acceleration;
	Rectangle m_terrainBounds;
};

} // namespace DS2DEngine

#endif
