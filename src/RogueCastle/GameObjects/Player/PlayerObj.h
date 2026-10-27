/* Port of RogueCastle/src/Game Objects/Player/PlayerObj.cs (movement and
 * animation states so far; combat and spells come with their milestones).
 * Line references in the comments point at the C# file.
 */
#ifndef ROGUE_CASTLE_PLAYER_OBJ_H
#define ROGUE_CASTLE_PLAYER_OBJ_H

#include "../../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include "../../../DS2DEngine/Physics/PhysicsManager.h"
#include "../../Types/InputMapType.h"
#include "../../Types/PlayerPart.h"

namespace RogueCastle
{

class PlayerObj : public DS2DEngine::PhysicsObjContainer
{
public:
	/* PlayerObj.cs line 31: the state constants. */
	static const int STATE_IDLE = 0;
	static const int STATE_WALKING = 1;
	static const int STATE_JUMPING = 2;
	static const int STATE_HURT = 3;
	static const int STATE_DASHING = 4;
	static const int STATE_LEVELUP = 5;
	static const int STATE_BLOCKING = 6;
	static const int STATE_FLYING = 7;
	static const int STATE_TANOOKI = 8;
	static const int STATE_DRAGON = 9;

	PlayerObj();

	void Update(GameTime *gameTime) override;
	void ChangeSprite(const std::string &spriteName) override;
	void RoomTransitionReset();     /* PlayerObj.cs line 2504 */
	/* Simplified port of UpdatePlayerCollision (ground probe + drop
	 * through); needs the level's terrain list. */
	void UpdatePlayerCollision();
	void SetTerrain(const std::vector<DS2DEngine::IPhysicsObj *> *terrain) { m_attachedTerrain = terrain; }

	int State = STATE_IDLE;
	bool m_isTouchingGround = false;
	bool m_isJumping = false;
	bool m_collidingLeft = false;
	bool m_collidingRight = false;
	int m_doubleJumpCount = 0;
	/* Rune/equipment gates (PlayerObj.cs line 4030/4035): 0 for the
	 * default knight until the manor unlocks them. */
	int TotalDoubleJumps = 0;
	int TotalAirDashes = 0;
	float m_dropThroughGroundTimer = 0;
	float m_dropThroughGroundDuration = 0.1f;   /* PlayerObj.cs line 99 */
	const std::vector<DS2DEngine::IPhysicsObj *> *m_attachedTerrain = nullptr;

	/* Attack / dash state (the C# LogicSet machinery, PlayerObj.cs
	 * InitializeLogic line 276, compacted: the original only ever plays
	 * m_standingAttack3LogicSet and m_airAttackLS). */
	enum ActiveLS
	{
		LS_NONE,
		LS_STANDING_ATTACK3,
		LS_AIR_ATTACK
	};
	ActiveLS m_activeLS = LS_NONE;
	int m_lsStep = 0;                 /* step inside the active logic set */
	int m_attackNumber = 0;
	float m_attackCounter = 0;
	float m_dashCounter = 0;
	float m_dashCooldownCounter = 0;
	int m_airDashCount = 0;
	bool ControlsLocked = false;
	bool IsAirAttacking = false;
	float AttackAnimationDelay = 1 / 20.0f;
	float AttackDelay = 0;
	float ComboDelay = 1.5f;          /* PlayerObj.cs line 229 */
	bool CanAirDash() const { return TotalAirDashes > 0; }
	bool CanBlock() const { return false; }   /* Knight2 class only */

	void LockControls() { ControlsLocked = true; }
	void UnlockControls() { ControlsLocked = false; }
	bool IsLogicSetActive() const { return m_activeLS != LS_NONE; }
	void ExecuteLogicSet(ActiveLS ls);  /* m_currentLogicSet.Execute() */
	void UpdateLogicSet();             /* m_currentLogicSet.Update() */

	/* EVs (InitializeEV, PlayerObj.cs line 210). */
	float Speed = 0;
	float RunSpeedMultiplier = 0;
	float JumpHeight = 0;
	float DoubleJumpHeight = 0;
	float StepUp = 0;
	float DashSpeed = 0;
	float DashTime = 0;
	float DashCoolDown = 0;
	Vector2 KnockBack = Vector2(0, 0);
	Vector2 EnemyKnockBack = Vector2(0, 0);

	void InitializeEV();
	float TotalMovementSpeed();      /* PlayerObj.cs line 4055 */

private:
	void UpdateAnimationState();     /* PlayerObj.cs line 1502 */
	void UpdateEquipmentObjs();       /* part visibility (PlayerObj.cs 505) */

	DS2DEngine::SpriteObj *m_playerHead = nullptr;
	DS2DEngine::SpriteObj *m_playerLegs = nullptr;
};

} // namespace RogueCastle

#endif
