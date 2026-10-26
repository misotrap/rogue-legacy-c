/* Port of DS2DEngine/src/GameObjs/PhysicsObjContainer.cs: a container whose
 * children are PhysicsObjs; its terrain bounds are the union of theirs and
 * its collision boxes are shared with the children (added by them).
 */
#ifndef DS2D_PHYSICS_OBJ_CONTAINER_H
#define DS2D_PHYSICS_OBJ_CONTAINER_H

#include "ObjContainer.h"
#include "PhysicsObj.h"
#include <vector>

namespace DS2DEngine
{

class PhysicsObjContainer : public ObjContainer
{
public:
	PhysicsObjContainer();
	explicit PhysicsObjContainer(const std::string &spriteName);

	PhysicsObjContainer *AsPhysicsObjContainer() override { return this; }

	void UpdateCollisionBoxes();
	void UpdatePhysics(GameTime *gameTime);
	void CollisionResponseTerrain(CollisionBox *thisBox, CollisionBox *otherBox);

	Vector2 Acceleration() const { return m_acceleration; }
	void Acceleration(const Vector2 &value) { m_acceleration = value; }
	float AccelerationX() const { return m_acceleration.X; }
	void AccelerationX(float value) { m_acceleration.X = value; }
	float AccelerationY() const { return m_acceleration.Y; }
	void AccelerationY(float value) { m_acceleration.Y = value; }

	bool IsWeighted = true;
	bool IsCollidable = true;
	bool CollidesLeft = true;
	bool CollidesRight = true;
	bool CollidesTop = true;
	bool CollidesBottom = true;
	bool AccelerationXEnabled = true;
	bool AccelerationYEnabled = true;
	bool DisableGravity = false;
	bool DisableAllWeight = false;
	bool DisableHitboxUpdating = false;

	Rectangle TerrainBounds() const { return m_terrainBounds; }
	const std::vector<CollisionBox *> &CollisionBoxes() const { return m_collisionBoxes; }
	/* Called by the children when they create shared boxes. */
	void AddSharedCollisionBox(CollisionBox *box) { m_collisionBoxes.push_back(box); }

protected:
	void BuildFromCharData(const std::string &spriteName) override;

	std::vector<CollisionBox *> m_collisionBoxes;   /* shared with children */
	Vector2 m_acceleration;
	Rectangle m_terrainBounds;
};

} // namespace DS2DEngine

#endif
