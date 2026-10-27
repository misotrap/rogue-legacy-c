/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj.cs (subset): the
 * common enemy flow - engage radii, facing, cooldown, movement, room
 * clamping, ground probe, taking damage, knockback, death.
 *
 * The C# drives each enemy with LogicBlocks; the port keeps the same EV
 * constants, states and probability tables but executes the actions in a
 * compact per-enemy action machine (see the EnemyObj_* subclasses).
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_H
#define ROGUE_CASTLE_ENEMY_OBJ_H

#include "../../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include <string>
#include <vector>

namespace RogueCastle
{

class PlayerObj;
class ProceduralLevelScreen;

class EnemyObj : public DS2DEngine::PhysicsObjContainer
{
public:
	/* EnemyObj.cs line 5: the engage states. */
	static const int STATE_WANDER = 0;
	static const int STATE_ENGAGE = 1;
	static const int STATE_PROJECTILE_ENGAGE = 2;
	static const int STATE_MELEE_ENGAGE = 3;

	/* GameTypes.EnemyDifficulty */
	enum EnemyDifficulty { BASIC = 0, ADVANCED = 1, EXPERT = 2, MINIBOSS = 3 };

	EnemyObj(const std::string &spriteName, PlayerObj *target, EnemyDifficulty difficulty);

	void Initialize();
	void Update(GameTime *gameTime) override;
	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer);
	virtual void Kill(bool giveXP = true);
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

	int DealsDamage() const override { return (int) Damage; }
	using DS2DEngine::PhysicsObjContainer::Visible;   /* setter + getter */
	void SetPlayerTarget(PlayerObj *target) { m_target = target; }
	void SetLevelScreen(ProceduralLevelScreen *levelScreen) { m_levelScreen = levelScreen; }
	void SetTerrain(const std::vector<DS2DEngine::IPhysicsObj *> *terrain) { m_attachedTerrain = terrain; }
	void ResetState();                 /* EnemyObj.cs line 1113 */

	/* EVs, set by InitializeBaseEV + the per-enemy InitializeEV. */
	float Speed = 1;
	float MaxHealth = 10;
	float CurrentHealth = 10;
	float Damage = 5;
	float ProjectileDamage = 5;
	float XPValue = 0;
	float EngageRadius = 400;
	float ProjectileRadius = 200;
	float MeleeRadius = 50;
	float CooldownTime = 0;
	float InvincibilityTime = 0.4f;
	float ProjectileSpeed = 0;
	float TurnSpeed = 0;
	float JumpHeight = 0;
	int MinMoneyDropAmount = 1;
	int MaxMoneyDropAmount = 1;
	float MoneyDropChance = 0.50f;
	Vector2 KnockBack = Vector2(0, 0);
	Vector2 ProjectileScale = Vector2(1, 1);
	bool AlwaysFaceTarget = false;
	bool CanBeKnockedBack = true;
	bool CanFallOffLedges = false;
	bool DropsItem = true;
	bool GivesLichHealth = true;
	bool NonKillable = false;
	std::string Name;
	int Type = 0;           /* EnemyType.h value */
	int Level = 0;          /* room level scaling (TODO with the castle) */
	int State = STATE_WANDER;
	float DistanceToPlayer = 0;
	bool IsKilled = false;
	bool IsPaused = false;
	bool IsInvincible() const { return m_invincibleCounter > 0 || m_invincibleCounterProjectile > 0; }

	PlayerObj *Target() const { return m_target; }
	EnemyDifficulty DifficultyType = BASIC;

protected:
	/* The per-enemy behaviour (RunBasicLogic etc. in the C#). */
	virtual void InitializeEV() = 0;
	virtual void RunBasicLogic() = 0;
	virtual void RunAdvancedLogic() { RunBasicLogic(); }
	virtual void RunExpertLogic() { RunAdvancedLogic(); }
	virtual void RunMinibossLogic() { RunExpertLogic(); }
	virtual void UpdateEnemy(GameTime *gameTime) { (void) gameTime; }
	/* The compact replacement of the C# LogicBlock: an enemy 'action'
	 * (walk, throw, jump...) runs for a while; a new action is only picked
	 * when none is active and the cooldown is over. */
	bool m_actionActive = false;
	float m_actionTimer = 0;      /* remaining time of the current action */
	int m_actionPhase = 0;        /* step inside multi-phase actions */

	/* Checks the terrain under the feet (simplified CheckGroundCollision)
	 * and turns the enemy around at ledges when it cannot fall off them. */
	void CheckGroundCollision();

	/* Blink (the red damage flash): flips the tint for blinkTime. */
	void Blink(Color colour, float blinkTime);

	PlayerObj *m_target = nullptr;
	ProceduralLevelScreen *m_levelScreen = nullptr;
	const std::vector<DS2DEngine::IPhysicsObj *> *m_attachedTerrain = nullptr;
	bool m_isTouchingGround = false;
	float m_invincibleCounter = 0;
	float m_invincibleCounterProjectile = 0;
	float m_cooldownTimer = 0;
	bool m_runCooldown = false;
	float m_initialDelayCounter = 0;
	bool m_wasFacingLeft = false;
	Vector2 m_resetPos = Vector2(0, 0);
	std::string m_resetSpriteName;
	/* Blink support: base tint + flash timer (the C# blinks red). */
	Color m_baseTint = Color(255, 255, 255);
	Color m_blinkColour = Color(255, 255, 255);
	float m_blinkCounter = 0;

private:
	void InitializeBaseEV();   /* EnemyObj.cs line 116 */
};

} // namespace RogueCastle

#endif
