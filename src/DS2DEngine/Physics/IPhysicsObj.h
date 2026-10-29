/* Port of DS2DEngine/src/Physics/IPhysicsObj.cs: the physics interface.
 * Weighted containers (player, enemies) and static terrain both implement
 * it, so PhysicsManager can run the same pair loop as the original.
 */
#ifndef DS2D_IPHYSICS_OBJ_H
#define DS2D_IPHYSICS_OBJ_H

#include "../GameObjs/CollisionBox.h"
#include "../Types.h"
#include <string>
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
	/* No RTTI (AGENTS.md): the enemies use this to tell the player's
	 * spell projectiles (which use the projectile invincibility counter
	 * and their own damage) from the sword. */
	virtual bool IsSpellProjectile() const { return false; }
	/* EnergonProjectileObj.CollisionResponse: the hero's box (its type) met
	 * this shot; true when the shot is sent back at its shooter */
	virtual bool Reflect(int playerBoxType, bool airAttacking, bool blocking) { (void) playerBoxType; (void) airAttacking; (void) blocking; return false; }
	/* What killed the hero (GameOverScreen): the enemy's name, "" for a
	 * shot without a shooter, nullptr when it is not a killer. boss: a
	 * miniboss or the last boss, "terrassé par" in the original. */
	virtual const std::string *KillerName(bool &boss) const { (void) boss; return nullptr; }
	/* The SpellType of a spell projectile (0 otherwise), read by the
	 * enemies for the Barbarian shout knockback. */
	virtual uint8_t ProjectileSpell() const { return 0; }
	/* The enemy was damaged by this object: the projectiles that die
	 * with their enemy destroy themselves here. */
	virtual void OnEnemyHit() {}
	/* The Dementia trait's imaginary enemies (and their shots) hurt no one */
	virtual bool IsHallucination() const { return false; }
};

} // namespace DS2DEngine

#endif
