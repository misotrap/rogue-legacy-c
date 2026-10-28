/* Port of RogueCastle/src/Game Objects/ProjectileObj.cs: a weighted sprite
 * with a weapon box. Enemy projectiles damage the player; the player's
 * spells (CollisionTypeTag = 2) damage the enemies. The per-spell behaviour
 * (boomerang, bounce, homing crows, damage shield, laser...) is Update /
 * CollisionResponse / ActivateEffect below, from ProjectileObj.cs.
 */
#ifndef ROGUE_CASTLE_PROJECTILE_OBJ_H
#define ROGUE_CASTLE_PROJECTILE_OBJ_H

#include "../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include "../Types/SpellType.h"

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
	bool IsSpellProjectile() const override { return true; }
	uint8_t ProjectileSpell() const override { return Spell; }
	/* ProjectileObj.cs line 697: DestroysWithEnemy. */
	void OnEnemyHit() override
	{
		if (DestroysWithEnemy && !IsDying)
			KillProjectile();
	}
	/* The bone has no terrain box; its weapon box bounds stand in for the
	 * active-bounds filter and the out-of-play check. */
	Rectangle TerrainBounds() const override { return Bounds(); }
	int Damage = 5;
	float RotationSpeed = 0;   /* degrees per 1/60 s */
	bool DestroysWithTerrain = false;
	bool IsKilled = false;
	/* Set to false right before the level screen frees the projectile;
	 * the enemies that hold one (the flail ball, the wizards' summons)
	 * check it so they never touch freed memory. */
	bool IsAlive = true;
	/* A held projectile (the flail ball, a channel marker) is owned by
	 * an enemy: the level screen must not free it on a room transition.
	 * The owner releases it (IsKilled, then nulls its pointer). */
	bool Held = false;
	/* ProjectileObj.cs: projectiles are given a lifespan; 0 = live until
	 * they hit something or leave play. */
	float LifeSpan = 0;

	/* The spell behaviours (ProjectileObj.cs fields). */
	uint8_t Spell = 0;
	float AltX = 0, AltY = 0;      /* per-spell scratch values */
	float Orientation = 0;          /* radians, for homing/bounce */
	float TurnSpeed = 0;
	bool ChaseTarget = false;
	class EnemyObj *Target = nullptr;
	bool IgnoreBoundsCheck = false;
	bool CollidesWithTerrainFlag = false;
	bool CollidesWith1Ways = false;
	bool DestroysWithEnemy = false;
	bool LockPosition = false;
	bool IsDying = false;
	float BlinkTime = 0;
	/* ProjectileObj.cs line 562 (ActivateEffect). */
	void ActivateEffect();
	void KillProjectile() { IsKilled = true; Visible(false); IsCollidableFlag = false; }

	/* The source (for the damage return and the debug); not owned. */
	class EnemyObj *Source = nullptr;
	bool Demented = false;   /* fired by a hallucination */
	bool IsHallucination() const override { return Demented; }
	/* The player who cast the spell; not owned. */
	class PlayerObj *SourcePlayer = nullptr;

private:
	float m_speed = 0;
	float m_elapsedLifeSpan = 0;
	void TurnToFace(Vector2 facePosition, float turnSpeed, float elapsed);
	void MoveByHeading(float elapsed);
};

} // namespace RogueCastle

#endif
