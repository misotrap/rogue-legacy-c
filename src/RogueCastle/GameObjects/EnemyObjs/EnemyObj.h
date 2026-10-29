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
	virtual void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer);
	virtual void Kill(bool giveXP = true);
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

	int DealsDamage() const override { return (int) Damage; }
	using DS2DEngine::PhysicsObjContainer::Visible;   /* setter + getter */
	void SetPlayerTarget(PlayerObj *target) { m_target = target; }
	void SetLevelScreen(ProceduralLevelScreen *levelScreen) { m_levelScreen = levelScreen; }
	void SetTerrain(const std::vector<DS2DEngine::IPhysicsObj *> *terrain) { m_attachedTerrain = terrain; }
	virtual void ResetState();       /* EnemyObj.cs line 1113 */

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
	/* EnemyObj.cs line 136: the money raise per enemy level. ADDITIVE */
	float MinMoneyGainPerLevel = 0.23f;
	float MaxMoneyGainPerLevel = 0.29f;
	float MoneyDropChance = 0.50f;
	Vector2 KnockBack = Vector2(0, 0);
	Vector2 ProjectileScale = Vector2(1, 1);
	bool AlwaysFaceTarget = false;
	bool CanBeKnockedBack = true;
	bool CanFallOffLedges = false;
	bool DropsItem = true;
	bool GivesLichHealth = true;
	bool NonKillable = false;
	bool LockFlip = false;   /* keeps the placed Flip (EnemyObj.cs line 301) */
	bool Demented = false;   /* IsDemented: a Dementia hallucination */
	bool IsHallucination() const override { return Demented; }
	std::string Name;
	std::string Tag;        /* the room object's Tag (delay:2.0, speed:600...) */
	int Type = 0;           /* EnemyType.h value */
	int Level = 0;          /* from the room level (ApplyLevel) */
	bool Procedural = false;   /* placed by an orb: promoted with the level */
	/* ProceduralLevelScreen.InitializeEnemies + the per-level gains
	 * (EnemyObj.cs lines 200 and 1222): once, after Initialize */
	void ApplyLevel(int level, bool bossRoom);
	/* the per-level gains alone, from the freshly initialized stats */
	void ApplyLevelGains(int level);
	int State = STATE_WANDER;
	float DistanceToPlayer = 0;
	bool IsKilled = false;
	bool IsPaused = false;
	/* EnemyObj.cs line 1015: the time stop freezes it (no logic, no
	 * weight, no animation) */
	void PauseEnemy() { if (IsPaused || IsKilled) return; IsPaused = true; DisableAllWeightFlag = true; PauseAnimation(); }
	void UnpauseEnemy() { if (!IsPaused) return; IsPaused = false; DisableAllWeightFlag = false; if (!IsKilled) ResumeAnimation(); }
	bool IsInvincible() const { return m_invincibleCounter > 0 || m_invincibleCounterProjectile > 0; }

	PlayerObj *Target() const { return m_target; }
	/* C# InitialLogicDelay: the enemy waits this long before acting. */
	void InitialLogicDelay(float value) { m_initialDelayCounter = value; }
	/* EnemyObj.cs line 1140: reads a "key:value" pair out of Tag. */
	float ParseTagToFloat(const std::string &key);
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
	/* The base ends the action when ActionComplete() says so; single-phase
	 * actions (walks) just use the timer, multi-phase attacks (windup,
	 * throw, recover) override it and finish on their last phase. */
	virtual bool ActionComplete() { return m_actionTimer <= 0; }

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
