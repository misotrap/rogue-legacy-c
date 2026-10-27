/* Port of DS2DEngine/src/GameObjs/PhysicsObj.cs: a sprite with acceleration,
 * gravity flags and collision boxes built from the current frame's hitboxes
 * (the 'R' records of the .spr files).
 */
#ifndef DS2D_PHYSICS_OBJ_H
#define DS2D_PHYSICS_OBJ_H

#include "SpriteObj.h"
#include "CollisionBox.h"
#include <vector>

namespace DS2DEngine
{

class PhysicsObj : public SpriteObj
{
public:
	explicit PhysicsObj(const std::string &spriteName);

	PhysicsObj *AsPhysicsObj() override { return this; }

	void UpdatePhysics(GameTime *gameTime);
	void UpdateCollisionBoxes();
	void AddCollisionBox(int xPos, int yPos, int width, int height, int hitboxType);
	void ClearCollisionBoxes();

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

	~PhysicsObj() override;

protected:
	Vector2 m_acceleration;
	std::vector<CollisionBox *> m_collisionBoxes;
	Rectangle m_terrainBounds;

	friend class PhysicsObjContainer;
};

} // namespace DS2DEngine

#endif
