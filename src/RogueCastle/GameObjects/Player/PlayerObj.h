/* Port of RogueCastle/src/Game Objects/Player/PlayerObj.cs (movement and
 * animation states so far; combat and spells come with their milestones).
 * Line references in the comments point at the C# file.
 */
#ifndef ROGUE_CASTLE_PLAYER_OBJ_H
#define ROGUE_CASTLE_PLAYER_OBJ_H

#include "../../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include "../../Save/SaveGame.h"
#include "../../../DS2DEngine/Physics/PhysicsManager.h"
#include "../../Types/InputMapType.h"
#include "../../Types/PlayerPart.h"
#include "../../Types/ClassType.h"
#include "../TerrainObj.h"

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
	void SetSlopes(const std::vector<Slope> *slopes) { m_slopes = slopes; }
	void SetHazards(const std::vector<Hazard> *hazards) { m_hazards = hazards; }
	void SetLevelScreen(class ProceduralLevelScreen *levelScreen) { m_levelScreen = levelScreen; }
	class ProceduralLevelScreen *LevelScreen() const { return m_levelScreen; }

	/* PlayerObj.cs line 2621 (CastSpell): the spells. activateSecondary
	 * is the trait/dragon variant; the SpellSword megaSpell comes with
	 * the class powers. */
	void CastSpell(bool activateSecondary, bool megaSpell = false);
	/* PlayerObj.cs line 3331 (CastDaggers): one rapid dagger throw. */
	void CastDaggers(bool randomize);
	/* The class powers (PlayerObj.cs lines 1005-1050): the block of the
	 * upgraded knight, the SpellSword mega spell, the Lich conversion,
	 * the assassin stealth, the Wizard spell swap, the Ninja teleport,
	 * the Barbarian shout, the Traitor axes. */
	void CastClassPower();            /* PLAYER_BLOCK just pressed */
	void ConvertHPtoMP();             /* PlayerObj.cs line 3000 */
	void ActivateAssassinAbility();   /* PlayerObj.cs line 3035 */
	void DisableAssassinAbility();    /* PlayerObj.cs line 3048 */
	void SwapSpells();                /* PlayerObj.cs line 3056 */
	void NinjaTeleport();             /* PlayerObj.cs line 3086 */
	void CastFuhRohDah();             /* PlayerObj.cs line 3229 */
	void ThrowAxeProjectiles();       /* PlayerObj.cs line ~3320 */
	/* The wall ahead in the facing direction (NinjaTeleport and the
	 * displacer, ProjectileObj.cs RunDisplacerEffect). */
	float ClosestWallDistance() const;
	/* PlayerObj.cs line 2978 (Translocate): the teleport of the quantum
	 * translocater and the displacer. */
	void Translocate(Vector2 position);
	bool CastingDamageShield() const { return m_damageShieldCast; }

	/* PlayerObj.cs HitPlayer (line 2245): damage, knockback, invincibility
	 * and the hurt state; death TODO with the death animation. */
	void HitPlayer(int damage, Vector2 attackPosition);
	/* The last thing that hurt the hero, for the game over text:
	 * 0 unknown, 1 an enemy, 2 a boss, 3 a shot without a shooter, 4 a trap */
	int KillerKind = 0;
	std::string KillerName;
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

	/* Health / damage (InitializeEV, PlayerObj.cs line 216). The real
	 * stats come from the save (Game.PlayerStats in the C#): the class,
	 * the manor skills, the equipment and the bonus drops. Until
	 * SetPlayerStats is called the base knight values apply. */
	void SetPlayerStats(Save::GameState *stats) { m_stats = stats; UpdateEquipmentObjs(); UpdateEquipmentColours(); RefreshAbilities(); }
	void UpdateEquipmentColours();   /* PlayerObj.cs line 2558 */
	Save::GameState *Stats() const { return m_stats; }

	float CurrentHealth = 100;
	float CurrentMana = 0;
	int TotalDamage() const;          /* PlayerObj.cs line 3561 (Damage) */
	/* IDealsDamageObj: what the enemy takes when the sword (or a spell
	 * projectile owned by the player) hits it. */
	int DealsDamage() const override { return TotalDamage(); }
	int TotalMagicDamage() const;    /* PlayerObj.cs line 3589 */
	float TotalArmor() const;        /* PlayerObj.cs line 4045 */
	float MaxHealth() const;         /* PlayerObj.cs line 3670 */
	float MaxMana() const;           /* PlayerObj.cs line 3631 */
	int MaxWeight() const;           /* PlayerObj.cs line 3838 */
	float TotalCritChance() const;      /* PlayerObj.cs line 3951 */
	float TotalCriticalDamage() const;  /* line 3968 */
	int TotalVampBonus() const;         /* line 4014: hp per kill */
	int ManaGain() const;               /* line 3920: mp per kill */
	/* The double jump and air dash runes and equipment (line 4030) */
	void RefreshAbilities();
	/* EnemyObj.Kill (line 884): vampirism, mana, the Lich's max hp */
	void OnEnemyKilled(int enemyLevel, bool givesLichHealth);
	float BaseInvincibilityTime = 1.0f;
	float AirAttackKnockBack = 1425;              /* PlayerEV */
	int NumSequentialAttacks = 0;
	bool IsKilled = false;
	float m_invincibleCounter = 0;
	/* Damage blink (the C# blinks red through the OutlineColour). */
	void Blink(Color colour, float blinkTime);
	float m_blinkCounter = 0;
	Color m_blinkColour = Color(255, 0, 0);
	DS2DEngine::SpriteObj *m_swearBubble = nullptr;   /* Tourettes */
	float m_swearBubbleCounter = 0;
	bool m_onOneWay = false;
	float m_prevBottom = 0;   /* the feet before this frame's move */   /* standing on a platform open underneath */
	void DrawSwearBubble(DS2DEngine::Camera2D *camera);   /* Tourettes */
	Color m_partColour[PlayerPart::Light + 1];   /* the armour tints (UpdateEquipmentColours) */

	Save::GameState *m_stats = nullptr;   /* Game.PlayerStats */
	int State = STATE_IDLE;
	ProceduralLevelScreen *m_levelScreen = nullptr;   /* AttachedLevel */
	/* The spell state (PlayerObj.cs fields). */
	float m_spellCastDelay = 0;
	bool m_damageShieldCast = false;
	bool m_megaDamageShieldCast = false;
	bool m_timeStopCast = false;
	/* The quantum translocater: the placed marker. */
	bool m_translocatorActive = false;
	Vector2 m_translocatorPos = Vector2(0, 0);
	/* The rapid daggers: the queued throws (Tween.RunFunction calls). */
	int m_rapidDaggerQueue = 0;
	float m_rapidDaggerTimer = 0;
	bool m_throwingAxes = false;   /* the Traitor's queued axes */
	/* The class powers (PlayerObj.cs fields). */
	bool m_assassinSpecialActive = false;
	float m_assassinDrainCounter = 0;
	float m_timeStopDrainCounter = 0;
	float m_damageShieldDrainCounter = 0;
	float m_ninjaTeleportDelay = 0;
	float m_blockInvincibleCounter = 0;
	float BlockInvincibleTime = 1.0f;   /* PlayerObj.cs line 268 */
	bool ForceInvincible = false;        /* the assassin special */
	bool m_isTouchingGround = false;
	bool m_isJumping = false;

	/* ChestConditionChecker reads these (C# properties). */
	bool IsTouchingGround() const { return m_isTouchingGround; }
	bool IsJumping() const { return m_isJumping; }

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
	const std::vector<Slope> *m_slopes = nullptr;
	const std::vector<Hazard> *m_hazards = nullptr;

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
	bool CanBlock() const
	{
		/* The upgraded knight only (PlayerObj.cs CanBlock property). */
		return m_stats != nullptr && m_stats->hero.classType == ClassType::Knight2;
	}

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
	/* PlayerObj.cs line 3609: the grounded swing knocks harder */
	Vector2 CurrentEnemyKnockBack() const { return m_activeLS == LS_STANDING_ATTACK3 ? Vector2(300, 360) : EnemyKnockBack; }
	int NumAirBounces = 0;      /* down-strikes chained without landing */
	float InvincibilityTime() const;   /* line 3697: + Invuln_Time_Up */
	float BlockManaDrain() const;
	float TotalGoldBonus() const;   /* line 3996 */

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
