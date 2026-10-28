/* Port of the code part of RogueCastle/src/EVs/SpellEV.cs. */
#include "SpellEVFuncs.h"
#include "../Types/SpellType.h"

namespace ST = SpellType;

/* The generated SpellEV.h is in the global namespace; the code part
 * follows it (the C# has both in one class). */
RogueCastle::ProjectileData SpellEV::GetProjData(uint8_t spellType, RogueCastle::PlayerObj *player)
{
	/* SpellEV.cs line 29: the projectile blueprint per spell. */
	RogueCastle::ProjectileData d;
	d.spriteName = "BoneProjectile_Sprite";
	d.source = player;

	switch (spellType)
	{
	case ST::Axe:
		d.spriteName = "SpellAxe_Sprite";
		d.angle = Vector2(-74, -74);
		d.speed = 1050;
		d.sourceAnchor = Vector2(50, -50);
		d.isWeighted = true;
		d.rotationSpeed = 10;
		d.scale = Vector2(3, 3);
		break;
	case ST::Dagger:
		d.spriteName = "SpellDagger_Sprite";
		d.angle = Vector2(0, 0);
		d.sourceAnchor = Vector2(50, 0);
		d.speed = 1750;
		d.collidesWithTerrain = true;
		d.destroysWithTerrain = true;
		d.scale = Vector2(2.5f, 2.5f);
		break;
	case ST::DragonFire:
		d.spriteName = "TurretProjectile_Sprite";
		d.angle = Vector2(0, 0);
		d.sourceAnchor = Vector2(50, 0);
		d.speed = 1100;
		d.lifespan = DRAGONFIRE_XVal;
		d.collidesWithTerrain = true;
		d.destroysWithTerrain = true;
		d.scale = Vector2(2.5f, 2.5f);
		break;
	case ST::DragonFireNeo:
		d.spriteName = "TurretProjectile_Sprite";
		d.angle = Vector2(0, 0);
		d.sourceAnchor = Vector2(50, 0);
		d.speed = 1750;
		d.lifespan = DRAGONFIRENEO_XVal;
		d.collidesWithTerrain = true;
		d.destroysWithTerrain = true;
		d.wrapProjectile = true;
		d.scale = Vector2(2.75f, 2.75f);
		break;
	case ST::TimeBomb:
		d.spriteName = "SpellTimeBomb_Sprite";
		d.angle = Vector2(-35, -35);
		d.speed = 500;
		d.sourceAnchor = Vector2(50, -50);
		d.isWeighted = true;
		d.startingRotation = 0;
		d.collidesWithTerrain = true;
		d.collidesWith1Ways = true;
		d.scale = Vector2(3, 3);
		break;
	case ST::Nuke:
		d.spriteName = "SpellNuke_Sprite";
		d.angle = Vector2(-65, -65);
		d.speed = 500;
		d.destroysWithEnemy = true;
		d.scale = Vector2(2, 2);
		break;
	case ST::Displacer:
		d.spriteName = "SpellDisplacer_Sprite";
		d.angle = Vector2(0, 0);
		d.speed = 0;
		d.collidesWithTerrain = true;
		d.collidesWith1Ways = true;
		d.scale = Vector2(2, 2);
		break;
	case ST::Boomerang:
		d.spriteName = "SpellBoomerang_Sprite";
		d.angle = Vector2(0, 0);
		d.sourceAnchor = Vector2(50, -10);
		d.speed = 790;
		d.rotationSpeed = 25;
		d.scale = Vector2(3, 3);
		break;
	case ST::DualBlades:
		d.spriteName = "SpellDualBlades_Sprite";
		d.angle = Vector2(-55, -55);
		d.sourceAnchor = Vector2(50, 30);
		d.speed = 1000;
		d.rotationSpeed = 30;
		d.scale = Vector2(2, 2);
		break;
	case ST::Close:
		d.spriteName = "SpellClose_Sprite";
		d.sourceAnchor = Vector2(120, -60);
		d.speed = 0;
		d.rotationSpeed = 0;
		d.scale = Vector2(2.5f, 2.5f);
		d.lockPosition = true;
		break;
	case ST::DamageShield:
		d.spriteName = "SpellDamageShield_Sprite";
		d.angle = Vector2(-65, -65);
		d.speed = 3.25f;
		d.target = nullptr;   /* the player, set at cast time */
		d.rotationSpeed = 0;
		d.scale = Vector2(3, 3);
		d.destroyOnRoomTransition = false;
		break;
	case ST::Bounce:
		d.spriteName = "SpellBounce_Sprite";
		d.angle = Vector2(-135, -135);
		d.speed = 785;
		d.startingRotation = -135;
		d.followArc = false;
		d.rotationSpeed = 20;
		d.sourceAnchor = Vector2(-10, -10);
		d.collidesWithTerrain = true;
		d.scale = Vector2(3.25f, 3.25f);
		break;
	case ST::Laser:
	case ST::RapidDagger:
		d.spriteName = "LaserSpell_Sprite";
		d.isCollidable = false;
		d.followArc = false;
		d.lockPosition = true;
		break;
	default:
		/* TimeStop, Translocator: no projectile. */
		break;
	}
	return d;
}

int SpellEV::GetManaCost(uint8_t spellType)
{
	/* SpellEV.cs line 234. */
	switch (spellType)
	{
	case ST::Dagger: return DAGGER_Cost;
	case ST::Axe: return AXE_Cost;
	case ST::TimeBomb: return TIMEBOMB_Cost;
	case ST::TimeStop: return TIMESTOP_Cost;
	case ST::Nuke: return NUKE_Cost;
	case ST::Translocator: return TRANSLOCATER_Cost;
	case ST::Displacer: return DISPLACER_Cost;
	case ST::Boomerang: return BOOMERANG_Cost;
	case ST::DualBlades: return DUAL_BLADES_Cost;
	case ST::Close: return CLOSE_Cost;
	case ST::DamageShield: return DAMAGE_SHIELD_Cost;
	case ST::Bounce: return BOUNCE_Cost;
	case ST::Laser: return LASER_Cost;
	case ST::DragonFire: return DRAGONFIRE_Cost;
	case ST::DragonFireNeo: return DRAGONFIRENEO_Cost;
	case ST::RapidDagger: return RAPIDDAGGER_Cost;
	}
	return 0;
}

float SpellEV::GetDamageMultiplier(uint8_t spellType)
{
	/* SpellEV.cs line 316. */
	switch (spellType)
	{
	case ST::Dagger: return DAGGER_Damage;
	case ST::Axe: return AXE_Damage;
	case ST::TimeBomb: return TIMEBOMB_Damage;
	case ST::TimeStop: return TIMESTOP_Damage;
	case ST::Nuke: return NUKE_Damage;
	case ST::Translocator: return TRANSLOCATER_Damage;
	case ST::Displacer: return DISPLACER_Damage;
	case ST::Boomerang: return BOOMERANG_Damage;
	case ST::DualBlades: return DUAL_BLADES_Damage;
	case ST::Close: return CLOSE_Damage;
	case ST::DamageShield: return DAMAGE_SHIELD_Damage;
	case ST::Bounce: return BOUNCE_Damage;
	case ST::Laser: return LASER_Damage;
	case ST::DragonFire: return DRAGONFIRE_Damage;
	case ST::DragonFireNeo: return DRAGONFIRENEO_Damage;
	case ST::RapidDagger: return RAPIDDAGGER_Damage;
	}
	return 0;
}

float SpellEV::GetXValue(uint8_t spellType)
{
	switch (spellType)
	{
	case ST::Dagger: return DAGGER_XVal;
	case ST::Axe: return AXE_XVal;
	case ST::TimeBomb: return TIMEBOMB_XVal;
	case ST::TimeStop: return TIMESTOP_XVal;
	case ST::Nuke: return NUKE_XVal;
	case ST::Translocator: return TRANSLOCATER_XVal;
	case ST::Displacer: return DISPLACER_XVal;
	case ST::Boomerang: return BOOMERANG_XVal;
	case ST::DualBlades: return DUAL_BLADES_XVal;
	case ST::Close: return CLOSE_XVal;
	case ST::DamageShield: return DAMAGE_SHIELD_XVal;
	case ST::Bounce: return BOUNCE_XVal;
	case ST::Laser: return LASER_XVal;
	case ST::DragonFire: return DRAGONFIRE_XVal;
	case ST::DragonFireNeo: return DRAGONFIRENEO_XVal;
	}
	return 0;
}

float SpellEV::GetYValue(uint8_t spellType)
{
	switch (spellType)
	{
	case ST::Dagger: return DAGGER_YVal;
	case ST::Axe: return AXE_YVal;
	case ST::TimeBomb: return TIMEBOMB_YVal;
	case ST::TimeStop: return TIMESTOP_YVal;
	case ST::Nuke: return NUKE_YVal;
	case ST::Translocator: return TRANSLOCATER_YVal;
	case ST::Displacer: return DISPLACER_YVal;
	case ST::Boomerang: return BOOMERANG_YVal;
	case ST::DualBlades: return DUAL_BLADES_YVal;
	case ST::Close: return CLOSE_YVal;
	case ST::DamageShield: return DAMAGE_SHIELD_YVal;
	case ST::Bounce: return BOUNCE_YVal;
	case ST::Laser: return LASER_YVal;
	case ST::DragonFire: return DRAGONFIRE_YVal;
	case ST::DragonFireNeo: return DRAGONFIRENEO_YVal;
	}
	return 0;
}

