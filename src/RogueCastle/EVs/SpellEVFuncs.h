/* Port of the code part of RogueCastle/src/EVs/SpellEV.cs (GetProjData,
 * GetManaCost, GetDamageMultiplier, GetXValue, GetYValue) and of the
 * ProjectileData class of ProjectileManager.cs: the projectile blueprint
 * each spell fires. The constants live in the generated SpellEV.h. */
#ifndef ROGUE_CASTLE_SPELL_EV_FUNCS_H
#define ROGUE_CASTLE_SPELL_EV_FUNCS_H

#include "SpellEV.h"
#include "../../DS2DEngine/Types.h"
#include <string>

/* ProjectileManager.cs line 393 (ProjectileData). */
namespace RogueCastle
{

/* ProjectileManager.cs line 393 (ProjectileData). */
struct ProjectileData
{
	std::string spriteName;
	Vector2 angle = Vector2(0, 0);        /* degrees, X..Y random range */
	float angleOffset = 0;
	Vector2 sourceAnchor = Vector2(0, 0);
	float speed = 0;                       /* Speed.X (Y unused) */
	float lifespan = 0;
	bool isWeighted = false;
	float rotationSpeed = 0;
	float startingRotation = 0;
	bool followArc = true;
	bool collidesWithTerrain = false;
	bool collidesWith1Ways = false;
	bool destroysWithTerrain = false;
	bool destroysWithEnemy = false;
	bool chaseTarget = false;
	bool showIcon = false;
	bool lockPosition = false;
	bool isCollidable = true;
	bool destroyOnRoomTransition = true;
	bool wrapProjectile = false;
	int damage = 0;
	Vector2 scale = Vector2(1, 1);
	class EnemyObj *target = nullptr;
	class PlayerObj *source = nullptr;
};

}

/* The generated SpellEV.h (the constants) is in the global namespace;
 * the code part follows it, like the C# has both in one class. */
namespace SpellEV
{
	/* SpellEV.cs line 29. */
	RogueCastle::ProjectileData GetProjData(uint8_t spellType, RogueCastle::PlayerObj *player);
	/* SpellEV.cs line 234. */
	int GetManaCost(uint8_t spellType);
	/* SpellEV.cs line 316. */
	float GetDamageMultiplier(uint8_t spellType);
	/* SpellEV.cs line 357/396. */
	float GetXValue(uint8_t spellType);
	float GetYValue(uint8_t spellType);
}
#endif
