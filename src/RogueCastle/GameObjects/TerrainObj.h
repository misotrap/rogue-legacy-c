/* Port of RogueCastle/src/Game Objects/TerrainObj.cs: a solid rectangle
 * (the CollHullObj of a room template). Weightless and collidable, with a
 * terrain box over its whole rectangle; a fall-through platform (top only)
 * has its collision height halved like the original.
 */
#ifndef ROGUE_CASTLE_TERRAIN_OBJ_H
#define ROGUE_CASTLE_TERRAIN_OBJ_H

#include "../../DS2DEngine/GameObjs/GameObj.h"
#include "../../DS2DEngine/GameObjs/CollisionBox.h"
#include "../../DS2DEngine/Physics/IPhysicsObj.h"
#include <vector>

namespace RogueCastle
{

class TerrainObj : public DS2DEngine::GameObj, public DS2DEngine::IPhysicsObj
{
public:
	TerrainObj(int width, int height);

	/* TerrainObj.cs PopulateFromXMLReader. */
	void Populate(int x, int y, int width, int height,
	              bool collidesTop, bool collidesBottom, bool collidesLeft, bool collidesRight);

	void Draw(DS2DEngine::Camera2D *camera) override;

	/* --- IPhysicsObj (weightless: only the collision pass reads these) --- */
	bool Visible() const override { return GameObj::Visible(); }
	bool DisableAllWeight() const override { return true; }   /* weightless anyway */
	bool IsWeighted() const override { return false; }
	bool AccelerationXEnabled() const override { return false; }
	bool AccelerationYEnabled() const override { return false; }
	bool DisableGravity() const override { return true; }
	float AccelerationX() const override { return 0; }
	void AccelerationX(float) override {}
	float AccelerationY() const override { return 0; }
	void AccelerationY(float) override {}
	void UpdatePhysics(GameTime *) override {}
	bool DisableHitboxUpdating() const override { return true; }
	void UpdateCollisionBoxes() override {}
	bool IsCollidable() const override { return true; }
	int CollisionTypeTag() const override { return 1; }   /* GameTypes.CollisionType_WALL */
	const std::vector<DS2DEngine::CollisionBox *> &CollisionBoxes() const override { return m_collisionBoxes; }
	Rectangle TerrainBounds() const override
	{
		return Rectangle((int) X(), (int) Y(), _width, _height);
	}
	bool CollidesTop() const override { return m_collidesTop; }
	bool CollidesBottom() const override { return m_collidesBottom; }
	bool CollidesLeft() const override { return m_collidesLeft; }
	bool CollidesRight() const override { return m_collidesRight; }
	void CollisionResponse(DS2DEngine::CollisionBox *, DS2DEngine::CollisionBox *, int) override
	{
		/* Walls never move (PhysicsObjContainer.CollisionResponse does the
		 * pushing, and only for weighted objects). */
	}

	~TerrainObj() override;

private:
	std::vector<DS2DEngine::CollisionBox *> m_collisionBoxes;
	bool m_collidesTop = true, m_collidesBottom = true, m_collidesLeft = true, m_collidesRight = true;
};

} // namespace RogueCastle

#endif
