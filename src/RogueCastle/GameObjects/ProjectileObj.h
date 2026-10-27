/* Port of RogueCastle/src/Game Objects/ProjectileObj.cs (subset): a
 * weighted sprite with a weapon box that damages the player on contact
 * (the bone the skeleton throws). The full class handles spells, bouncing,
 * pooling; the port grows with the spells milestone.
 */
#ifndef ROGUE_CASTLE_PROJECTILE_OBJ_H
#define ROGUE_CASTLE_PROJECTILE_OBJ_H

#include "../../DS2DEngine/GameObjs/PhysicsObjContainer.h"

namespace RogueCastle
{

class ProjectileObj : public DS2DEngine::PhysicsObjContainer
{
public:
	ProjectileObj(const std::string &spriteName, Vector2 position,
	              Vector2 angle, float speed, int damage,
	              Vector2 scale, bool weighted);

	void Update(GameTime *gameTime) override;
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

	int DealsDamage() const override { return Damage; }
	int Damage = 5;
	float RotationSpeed = 0;   /* TODO C++: spinning blits */
	bool DestroysWithTerrain = false;
	bool IsKilled = false;

	/* The source (for the damage return and the debug); not owned. */
	class EnemyObj *Source = nullptr;

private:
	float m_speed = 0;
};

} // namespace RogueCastle

#endif
