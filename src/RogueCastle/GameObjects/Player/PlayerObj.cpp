/* Port of RogueCastle/src/Game Objects/Player/PlayerObj.cs. */
#include "PlayerObj.h"
#include "../../Game.h"
#include "../../../DS2DEngine/Consts.h"
#include "../../EVs/GameEV.h"
#include "../../EVs/PlayerEV.h"
#include "../../EVs/SpellEVFuncs.h"
#include "../../Types/ClassType.h"
#include "../../Types/TraitType.h"
#include "../../Types/SpellType.h"
#include "../ProjectileObj.h"
#include "../EnemyObjs/EnemyObj.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../../Lineage/Lineage.h"
#include "../../Skills/SkillTree.h"
#include "../../../Audio/Audio.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

PlayerObj::PlayerObj()
	: PhysicsObjContainer("PlayerIdle_Character")
{
	InitializeEV();

	/* PlayerObj.cs line 397 / 340. */
	Scale(Vector2(2, 2));
	ForceDraw(true);
	CollisionTypeTag(2);   /* GameTypes.CollisionType_PLAYER */
	IsWeightedFlag = true;
	IsCollidableFlag = true;

	m_playerHead = static_cast<SpriteObj *>(GetChildAt(PlayerPart::Head));
	m_playerLegs = static_cast<SpriteObj *>(GetChildAt(PlayerPart::Legs));
	if (NumChildren() == 0)
	{
		/* The character data was not found (sheet missing or name typo):
		 * fail loudly instead of crashing later. */
		fprintf(stderr, "PlayerObj: character data missing, is the sprite sheet loaded?\n");
		return;
	}
	UpdateEquipmentObjs();
	PlayAnimation(true);
}

void PlayerObj::InitializeEV()
{
	/* PlayerObj.cs line 210 (InitializeEV). */
	Speed = 500;
	RunSpeedMultiplier = 3.0f;
	JumpHeight = 1180;
	DoubleJumpHeight = 845;
	StepUp = 10;
	DashSpeed = 900.0f;
	DashTime = 0.325f;
	DashCoolDown = 0.25f;
	KnockBack = Vector2(300.0f, 450.0f);
	EnemyKnockBack = Vector2(90.0f, 90.0f);
	AnimationDelay(1 / 10.0f);   /* PlayerObj.cs line 236: 45 fps. */
}

float PlayerObj::TotalMovementSpeed()
{
	/* PlayerObj.cs line 4055: moveSpeed = Speed * (TotalMovementSpeedPercent
	 * + flightSpeedMod). The percent is 1 + equipment/runes/trait bonuses
	 * (all 0 for the default knight, see TotalMovementSpeedPercent). */
	return Speed * (1.0f + 0.0f);
}

void PlayerObj::ChangeSprite(const std::string &spriteName)
{
	PhysicsObjContainer::ChangeSprite(spriteName);
	UpdateEquipmentObjs();
}

void PlayerObj::UpdateEquipmentObjs()
{
	/* PlayerObj.cs line 526 (UpdateEquipmentObjs): for the default male
	 * knight without special items, the glasses, female parts, class extra
	 * and dragon wings are hidden. Hair IS visible (Baldness only). */
	if (NumChildren() < PlayerPart::NumHeadPieces + 11)
		return;   /* container smaller than expected: nothing to toggle */
	GetChildAt(PlayerPart::Glasses)->Visible(false);
	GetChildAt(PlayerPart::Boobs)->Visible(false);
	GetChildAt(PlayerPart::Bowtie)->Visible(false);
	GetChildAt(PlayerPart::Extra)->Visible(false);
	GetChildAt(PlayerPart::Wings)->Visible(false);
	GetChildAt(PlayerPart::Light)->Visible(false);
	if (GetChildAt(PlayerPart::Hair) != nullptr)
		GetChildAt(PlayerPart::Hair)->Visible(true);
}

void PlayerObj::ExecuteLogicSet(ActiveLS ls)
{
	/* The actions of m_standingAttack3LogicSet / m_airAttackLS
	 * (PlayerObj.cs InitializeLogic line 276), one step at a time:
	 * each PlayAnimation step completes before the next one runs. */
	m_activeLS = ls;
	m_lsStep = 0;
	UpdateLogicSet();
}

void PlayerObj::UpdateLogicSet()
{
	if (m_activeLS == LS_NONE)
		return;
	/* The current PlayAnimation step is still running. */
	if (IsAnimating() && m_lsStep > 0)
		return;

	if (m_activeLS == LS_STANDING_ATTACK3)
	{
		switch (m_lsStep)
		{
		case 0:   /* ChangeSprite + AnimationDelay + PlayAnimation(2, 4) */
			Audio::PlaySound("Player_Attack01", "Player_Attack02");   /* the attack LogicSets */
			ChangeSprite("PlayerAttacking3_Character");
			UpdateEquipmentObjs();
			AnimationDelay(AttackAnimationDelay);
			PlayAnimation(2, 4, false);
			break;
		case 1:   /* Delay(AttackDelay) then PlayAnimation("AttackStart","End") */
			PlayAnimation("AttackStart", "End", false);
			break;
		case 2:   /* ChangeProperty AnimationDelay back */
			AnimationDelay(1 / 10.0f);
			m_activeLS = LS_NONE;
			m_lsStep = 0;
			return;
		}
		m_lsStep++;
	}
	else if (m_activeLS == LS_AIR_ATTACK)
	{
		switch (m_lsStep)
		{
		case 0:   /* IsAirAttacking + ChangeSprite + AnimationDelay */
			IsAirAttacking = true;
			ChangeSprite("PlayerAirAttack_Character");
			UpdateEquipmentObjs();
			AnimationDelay(AttackAnimationDelay);
			PlayAnimation("Start", "Start", false);
			break;
		case 1:
			PlayAnimation("Frame 3 Test", "Frame 3 Test", false);
			break;
		case 2:
			PlayAnimation("Attack", "Attack", false);
			break;
		case 3:   /* Delay(AttackDelay) */
			PlayAnimation("Attack", "End", false);
			break;
		case 4:
			AnimationDelay(1 / 10.0f);
			IsAirAttacking = false;
			m_activeLS = LS_NONE;
			m_lsStep = 0;
			return;
		}
		m_lsStep++;
	}
}

void PlayerObj::Update(GameTime *gameTime)
{
	float elapsedSeconds = (float) gameTime->ElapsedGameTime;

	/* The spell counters (PlayerObj.cs lines 995-1020, 1405-1480): the
	 * abilities drain mana per MANA_OVER_TIME_TIC_RATE tick. */
	if (m_spellCastDelay > 0)
		m_spellCastDelay -= elapsedSeconds;
	if (m_ninjaTeleportDelay > 0)
		m_ninjaTeleportDelay -= elapsedSeconds;
	if (m_blockInvincibleCounter > 0)
		m_blockInvincibleCounter -= elapsedSeconds;
	if (m_damageShieldCast)
	{
		m_damageShieldDrainCounter += elapsedSeconds;
		if (m_damageShieldDrainCounter >= GameEV::MANA_OVER_TIME_TIC_RATE)
		{
			m_damageShieldDrainCounter = 0;
			CurrentMana -= m_megaDamageShieldCast
			                 ? GameEV::DAMAGESHIELD_ACTIVE_MANA_DRAIN * GameEV::SPELLSWORD_MANACOST_MOD
			                 : GameEV::DAMAGESHIELD_ACTIVE_MANA_DRAIN;
			if (CurrentMana <= 0)
			{
				CurrentMana = 0;
				m_damageShieldCast = false;
				m_megaDamageShieldCast = false;
			}
		}
	}
	if (m_assassinSpecialActive)
	{
		m_assassinDrainCounter += elapsedSeconds;
		if (m_assassinDrainCounter >= GameEV::MANA_OVER_TIME_TIC_RATE)
		{
			m_assassinDrainCounter = 0;
			CurrentMana -= GameEV::ASSASSIN_ACTIVE_MANA_DRAIN;
			if (CurrentMana <= 0)
				DisableAssassinAbility();
		}
	}
	if (m_timeStopCast)
	{
		m_timeStopDrainCounter += elapsedSeconds;
		if (m_timeStopDrainCounter >= GameEV::MANA_OVER_TIME_TIC_RATE)
		{
			m_timeStopDrainCounter = 0;
			CurrentMana -= GameEV::TIMESTOP_ACTIVE_MANA_DRAIN;
			if (CurrentMana <= 0)
			{
				CurrentMana = 0;
				if (m_levelScreen != nullptr)
					m_levelScreen->StopTimeStop();
				m_timeStopCast = false;
			}
		}
	}
	/* The rapid daggers (and the Traitor's axes): one throw every 0.05s
	 * (ThrowDaggerProjectiles, the Tween.RunFunction calls of
	 * PlayerObj.cs line 3321). */
	if (m_rapidDaggerQueue > 0)
	{
		m_rapidDaggerTimer -= elapsedSeconds;
		if (m_rapidDaggerTimer <= 0)
		{
			m_rapidDaggerQueue--;
			if (m_rapidDaggerQueue == 0)
				m_throwingAxes = false;
			m_rapidDaggerTimer = 0.05f;
			CastDaggers(m_rapidDaggerQueue < 4);
		}
	}

	/* PlayerObj.cs line 1254 (Update). */
	if (m_dropThroughGroundTimer > 0)
		m_dropThroughGroundTimer -= elapsedSeconds;
	if (m_invincibleCounter > 0)
		m_invincibleCounter -= elapsedSeconds;
	if (m_blinkCounter > 0)
		m_blinkCounter -= elapsedSeconds;
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->TextureColor(m_blinkCounter > 0 ? m_blinkColour : Color(255, 255, 255));

	if (!ControlsLocked)
	{
		/* Code for moving left and right (PlayerObj.cs line 798). */
		if (State != STATE_BLOCKING && State != STATE_TANOOKI)
		{
			bool left = Game::GlobalInput.Pressed(PLAYER_LEFT1) || Game::GlobalInput.Pressed(PLAYER_LEFT2);
			bool right = Game::GlobalInput.Pressed(PLAYER_RIGHT1) || Game::GlobalInput.Pressed(PLAYER_RIGHT2);
			if (left || right)
			{
				if (m_isTouchingGround)
					State = STATE_WALKING;

				if (right && (m_collidingRight == false || m_isTouchingGround))
				{
					HeadingX(1);
					CurrentSpeed(TotalMovementSpeed());
				}
				else if (left && !right && (m_collidingLeft == false || m_isTouchingGround))
				{
					HeadingX(-1);
					CurrentSpeed(TotalMovementSpeed());
				}
				else
					CurrentSpeed(0);

				/* PlayerObj.cs line 823: facing follows the input. */
				if (right)
					Flip(SpriteEffects::None);
				else if (left)
					Flip(SpriteEffects::FlipHorizontally);
			}
			else
			{
				if (m_isTouchingGround)
					State = STATE_IDLE;
				CurrentSpeed(0);
			}
		}

		/* Code for jumping and double jumping (PlayerObj.cs line 854). */
		if (State != STATE_BLOCKING && State != STATE_FLYING && State != STATE_TANOOKI)
		{
			if ((Game::GlobalInput.JustPressed(PLAYER_JUMP1) || Game::GlobalInput.JustPressed(PLAYER_JUMP2))
			    && m_isTouchingGround)
			{
				State = STATE_JUMPING;
				AccelerationY(-JumpHeight);
				m_isJumping = true;
				Audio::PlaySound("Player_Jump_04");   /* PlayerObj.cs line 867 */
			}
			else if ((Game::GlobalInput.JustPressed(PLAYER_JUMP1) || Game::GlobalInput.JustPressed(PLAYER_JUMP2))
			         && !m_isTouchingGround && m_doubleJumpCount < TotalDoubleJumps)
			{
				State = STATE_JUMPING;
				AccelerationY(-DoubleJumpHeight);
				m_isJumping = true;
				m_doubleJumpCount++;
				Audio::PlaySound("Player_DoubleJump");   /* PlayerObj.cs line 849 */
			}
		}

		/* Code for attacking (PlayerObj.cs line 926): only when no attack
		 * animation is running. */
		if (!IsLogicSetActive() && State != STATE_BLOCKING && State != STATE_TANOOKI
		    && Game::GlobalInput.JustPressed(PLAYER_ATTACK))
		{
			if (State == STATE_JUMPING)
			{
				/* In the air the same attack set plays; holding down aims
				 * it downward (the sprite is the same, PlayerObj.cs line
				 * 944). */
				ExecuteLogicSet(LS_AIR_ATTACK);
			}
			else
			{
				if (!m_isTouchingGround)
					CurrentSpeed(0);

				if (m_attackCounter > 0)
					m_attackNumber++;
				m_attackCounter = ComboDelay;
				/* The original simplified every combo hit to the same
				 * logic set (PlayerObj.cs line 963). */
				ExecuteLogicSet(LS_STANDING_ATTACK3);
				m_attackNumber = 0;
				m_attackCounter = 0;
			}
		}

		/* Code for casting spells (PlayerObj.cs line 1004). */
		if (Game::GlobalInput.JustPressed(PLAYER_SPELL) && m_spellCastDelay <= 0)
			CastSpell(false);

		/* Code for blocking (PlayerObj.cs line 756): the upgraded
		 * knight, while the block button is held. */
		if (Game::GlobalInput.Pressed(PLAYER_BLOCK) && CanBlock() && !IsLogicSetActive())
		{
			if (CurrentMana >= GameEV::KNIGHT_BLOCK_DRAIN)
			{
				if (m_isTouchingGround)
					CurrentSpeed(0);
				State = STATE_BLOCKING;
				if (Game::GlobalInput.JustPressed(PLAYER_BLOCK))
					Audio::PlaySound("Player_Block_Action");
			}
			else if (Game::GlobalInput.JustPressed(PLAYER_BLOCK))
				Audio::PlaySound("Error_Spell");
		}
		else if (State == STATE_BLOCKING)
		{
			/* The block ends with the button (PlayerObj.cs line 780). */
			State = m_isTouchingGround ? STATE_IDLE : STATE_JUMPING;
		}

		/* The class powers on the block button (PlayerObj.cs line 1011),
		 * outside the bonus rooms (not ported yet, they are fine). */
		if (Game::GlobalInput.JustPressed(PLAYER_BLOCK))
			CastClassPower();

		/* Code for dashing (PlayerObj.cs line 1109). */
		if (m_dashCooldownCounter <= 0
		    && (m_isTouchingGround || (!m_isTouchingGround && m_airDashCount < TotalAirDashes))
		    && State != STATE_BLOCKING && State != STATE_TANOOKI && CanAirDash())
		{
			int dashDir = 0;
			if (Game::GlobalInput.JustPressed(PLAYER_DASHLEFT))
				dashDir = -1;
			else if (Game::GlobalInput.JustPressed(PLAYER_DASHRIGHT))
				dashDir = 1;
			if (dashDir != 0)
			{
				m_airDashCount++;
				State = STATE_DASHING;
				AccelerationYFlag = false;
				m_dashCooldownCounter = DashCoolDown;
				m_dashCounter = DashTime;
				LockControls();
				CurrentSpeed(DashSpeed);
				HeadingX((float) dashDir);
				AccelerationY(0);
				m_activeLS = LS_NONE;   /* Stop the running attack. */
				AnimationDelay(1 / 10.0f);
			}
		}
	}

	/* PlayerObj.cs line 1359: the counters of the logic-set framework. */
	if (m_attackCounter > 0)
		m_attackCounter -= elapsedSeconds;
	else
		m_attackNumber = 0;

	UpdateLogicSet();

	if (m_dashCooldownCounter > 0)
		m_dashCooldownCounter -= elapsedSeconds;

	if (m_dashCounter > 0)
	{
		m_dashCounter -= elapsedSeconds;
		if (m_dashCounter <= 0 && State != STATE_HURT)
		{
			UnlockControls();
			AccelerationYFlag = true;
			m_airDashCount = 0;   /* reset by touching ground in the original */
		}
	}

	/* PlayerObj.cs line 1335: friction on knockback, then the position. */
	if (AccelerationX() < 0)
		AccelerationX(AccelerationX() + 200.0f * elapsedSeconds);
	X(X() + Heading().X * (CurrentSpeed() * elapsedSeconds));

	UpdatePlayerCollision();
	/* UpdateAnimationState only runs when no logic set is active
	 * (PlayerObj.cs line 1392). */
	if (!IsLogicSetActive())
		UpdateAnimationState();
}

void PlayerObj::UpdatePlayerCollision()
{
	/* Simplified port of UpdatePlayerCollision (PlayerObj.cs line 1600):
	 * the original casts a probe rect under the player to find the closest
	 * floor; here we test the terrain under the feet directly. */
	if (m_attachedTerrain == nullptr)
		return;

	Rectangle tb = TerrainBounds();
	Rectangle probe(tb.X + 1, tb.Bottom() - 1, tb.Width > 2 ? tb.Width - 2 : 1, 11);
	bool touching = false;
	bool onOneWay = false;
	for (const DS2DEngine::IPhysicsObj *terrain : *m_attachedTerrain)
	{
		if (!terrain->CollidesTop())
			continue;
		Rectangle t = terrain->TerrainBounds();
		if (!probe.Intersects(t))
			continue;
		/* New logic for determining whether you should fall through a
		 * one-way (PlayerObj.cs line 1739). */
		if (m_dropThroughGroundTimer > 0 && !terrain->CollidesBottom())
			continue;
		touching = true;
		onOneWay = !terrain->CollidesBottom();
		break;
	}

	if (touching && State != STATE_DRAGON)
	{
		/* Player just landed (PlayerObj.cs line 1795). */
		if (!m_isTouchingGround)
		{
			/* Reset any jumping states, i.e. air jump, air dash, etc. */
			m_isJumping = false;
			m_doubleJumpCount = 0;
			m_airDashCount = 0;
			if (State != STATE_BLOCKING && State != STATE_TANOOKI && State != STATE_DASHING)
				State = STATE_IDLE;

			/* Code for dropping through ground (PlayerObj.cs line 1812). */
			bool down = Game::GlobalInput.Pressed(PLAYER_DOWN1) || Game::GlobalInput.Pressed(PLAYER_DOWN2);
			bool jumpJustPressed = Game::GlobalInput.JustPressed(PLAYER_JUMP1) || Game::GlobalInput.JustPressed(PLAYER_JUMP2);
			if (down && jumpJustPressed && onOneWay && State != STATE_TANOOKI)
			{
				AccelerationY(0);
				Y(Y() + 15.0f);   /* must be 1 larger than the ground probe */
				m_isTouchingGround = false;
				m_isJumping = true;
				m_dropThroughGroundTimer = m_dropThroughGroundDuration;
			}
		}
		if (m_isTouchingGround == false && m_dropThroughGroundTimer <= 0)
			m_isTouchingGround = true;
	}
	else
		m_isTouchingGround = false;
}

void PlayerObj::RoomTransitionReset()
{
	/* PlayerObj.cs line 2504: only spell state is reset here; the port has
	 * no spells yet. */
}

void PlayerObj::Blink(Color colour, float blinkTime)
{
	m_blinkColour = colour;
	m_blinkCounter = blinkTime;
}

/* ---------------------------------------------------------------------------
 * The real stats (PlayerObj.cs): the class multipliers (lines 4116+),
 * the base values of InitializeEV (lines 219-263: BaseHealth 100,
 * BaseMana 100, MinDamage/MaxDamage 25, BaseMagicDamage 25, gains per
 * level 0) and the sums over the skills, the equipment and the bonus
 * drops. Game.PlayerStats is m_stats here.
 * ------------------------------------------------------------------------ */

static float ClassDamageGivenMultiplier(uint8_t klass)
{
	using namespace ClassType;
	switch (klass)
	{
	case Barbarian: case Barbarian2: return PlayerEV::BARBARIAN_DAMAGE_MOD;
	case Wizard: case Wizard2: return PlayerEV::MAGE_DAMAGE_MOD;
	case SpellSword: case SpellSword2: return PlayerEV::SPELLSWORD_DAMAGE_MOD;
	case Banker: case Banker2: return PlayerEV::BANKER_DAMAGE_MOD;
	case Ninja: case Ninja2: return PlayerEV::NINJA_DAMAGE_MOD;
	case Assassin: case Assassin2: return PlayerEV::ASSASSIN_DAMAGE_MOD;
	case Lich: case Lich2: return PlayerEV::LICH_DAMAGE_MOD;
	default: return PlayerEV::KNIGHT_DAMAGE_MOD;
	}
}

static float ClassMagicDamageGivenMultiplier(uint8_t klass)
{
	using namespace ClassType;
	switch (klass)
	{
	case Wizard: case Wizard2: return PlayerEV::MAGE_MAGICDAMAGE_MOD;
	case Lich: case Lich2: return PlayerEV::LICH_MAGICDAMAGE_MOD;
	default: return 1.0f;
	}
}

static float ClassTotalHPMultiplier(uint8_t klass)
{
	using namespace ClassType;
	switch (klass)
	{
	case Wizard: case Wizard2: return PlayerEV::MAGE_HEALTH_MOD;
	case Banker: case Banker2: return PlayerEV::BANKER_HEALTH_MOD;
	case Barbarian: case Barbarian2: return PlayerEV::BARBARIAN_HEALTH_MOD;
	case Ninja: case Ninja2: return PlayerEV::NINJA_HEALTH_MOD;
	case Assassin: case Assassin2: return PlayerEV::ASSASSIN_HEALTH_MOD;
	case SpellSword: case SpellSword2: return PlayerEV::SPELLSWORD_HEALTH_MOD;
	case Lich: case Lich2: return PlayerEV::LICH_HEALTH_MOD;
	default: return 1.0f;
	}
}

static float ClassTotalMPMultiplier(uint8_t klass)
{
	using namespace ClassType;
	switch (klass)
	{
	case Wizard: case Wizard2: return PlayerEV::MAGE_MANA_MOD;
	case Banker: case Banker2: return PlayerEV::BANKER_MANA_MOD;
	case Barbarian: case Barbarian2: return PlayerEV::BARBARIAN_MANA_MOD;
	case Ninja: case Ninja2: return PlayerEV::NINJA_MANA_MOD;
	case Assassin: case Assassin2: return PlayerEV::ASSASSIN_MANA_MOD;
	case SpellSword: case SpellSword2: return PlayerEV::SPELLSWORD_MANA_MOD;
	case Lich: case Lich2: return PlayerEV::LICH_MANA_MOD;
	default: return 1.0f;
	}
}

int PlayerObj::TotalDamage() const
{
	/* PlayerObj.cs line 3561: (RandomDamage + the attack skills) times
	 * the class multiplier; RandomDamage (line 3621) is the base 25
	 * (MinDamage == MaxDamage) plus the equipment, the strength drops
	 * and the per-level gain (0). */
	if (m_stats == nullptr)
		return 25;
	Equipment::Totals equip = m_stats->equipment.ComputeTotals();
	int damageDealt = (int) ((25 + equip.damage
	                         + m_stats->BonusStrength * GameEV::ITEM_STAT_STRENGTH_AMOUNT
	                         + m_stats->skills.ModifierAmount(Skills::SkillType::Attack_Up)
	                         + m_stats->skills.ModifierAmount(Skills::SkillType::Damage_Up_Final))
	                         * ClassDamageGivenMultiplier(m_stats->hero.classType));
	if (damageDealt < 1)
		damageDealt = 1;
	return damageDealt;
}

int PlayerObj::TotalMagicDamage() const
{
	/* PlayerObj.cs line 3589. */
	if (m_stats == nullptr)
		return 25;
	Equipment::Totals equip = m_stats->equipment.ComputeTotals();
	int intelligence = (int) ((25
	                           + m_stats->skills.ModifierAmount(Skills::SkillType::Magic_Damage_Up)
	                           + equip.magic
	                           + m_stats->BonusMagic * GameEV::ITEM_STAT_MAGIC_AMOUNT)
	                           * ClassMagicDamageGivenMultiplier(m_stats->hero.classType));
	if (intelligence < 1)
		intelligence = 1;
	return intelligence;
}

float PlayerObj::TotalArmor() const
{
	/* PlayerObj.cs line 4045. */
	if (m_stats == nullptr)
		return 0;
	Equipment::Totals equip = m_stats->equipment.ComputeTotals();
	return m_stats->skills.ModifierAmount(Skills::SkillType::Armor_Up)
	     + m_stats->BonusDefense * GameEV::ITEM_STAT_ARMOR_AMOUNT
	     + equip.armor;
}

float PlayerObj::MaxHealth() const
{
	/* PlayerObj.cs line 3670: (100 + equipment health + the health
	 * skills) times the class multiplier, plus the Lich health. The
	 * Dextrocardia trait swaps it with MaxMana (line 3673). */
	if (m_stats == nullptr)
		return 100;
	if (m_stats->hero.HasTrait(TraitType::Dextrocardia))
		return MaxMana();
	Equipment::Totals equip = m_stats->equipment.ComputeTotals();
	int maxHealth = (int) ((100 + equip.health
	                       + m_stats->BonusHealth * GameEV::ITEM_STAT_MAXHP_AMOUNT
	                       + m_stats->skills.ModifierAmount(Skills::SkillType::Health_Up)
	                       + m_stats->skills.ModifierAmount(Skills::SkillType::Health_Up_Final))
	                       * ClassTotalHPMultiplier(m_stats->hero.classType))
	                  + m_stats->LichHealth;
	if (maxHealth < 1)
		maxHealth = 1;
	return maxHealth;
}

float PlayerObj::MaxMana() const
{
	/* PlayerObj.cs line 3631 (Dextrocardia swaps it with MaxHealth). */
	if (m_stats == nullptr)
		return 100;
	if (m_stats->hero.HasTrait(TraitType::Dextrocardia))
		return MaxHealth();
	Equipment::Totals equip = m_stats->equipment.ComputeTotals();
	int maxMana = (int) ((100 + equip.mana
	                     + m_stats->BonusMana * GameEV::ITEM_STAT_MAXMP_AMOUNT
	                     + m_stats->skills.ModifierAmount(Skills::SkillType::Mana_Up)
	                     + m_stats->skills.ModifierAmount(Skills::SkillType::Mana_Up_Final))
	                     * ClassTotalMPMultiplier(m_stats->hero.classType))
	                    + m_stats->LichMana;
	if (maxMana < 1)
		maxMana = 1;
	return maxMana;
}

int PlayerObj::MaxWeight() const
{
	/* PlayerObj.cs line 3838: the base weight plus the Equip_Up skills
	 * plus the weight drops (EquipmentSystem::MaxWeight). */
	if (m_stats == nullptr)
		return Equipment::EquipmentSystem::MaxWeight(
			60, 0, 0, 0, GameEV::ITEM_STAT_WEIGHT_AMOUNT);
	return Equipment::EquipmentSystem::MaxWeight(
		60,
		m_stats->skills.ModifierAmount(Skills::SkillType::Equip_Up),
		m_stats->skills.ModifierAmount(Skills::SkillType::Equip_Up_Final),
		m_stats->BonusWeight, GameEV::ITEM_STAT_WEIGHT_AMOUNT);
}

void PlayerObj::HitPlayer(int damage, Vector2 attackPosition)
{
	/* PlayerObj.cs line 2245 (HitPlayer), subset: no demented/projectile
	 * special cases, no damage reduction (no equipment yet). */
	(void) attackPosition;
	Blink(Color(255, 0, 0), 0.1f);
	AccelerationYFlag = true;   /* in case the acceleration was disabled (dash) */
	UnlockControls();

	CurrentHealth -= damage;
	m_isJumping = false;

	if (CurrentHealth > 0)
	{
		/* Knockback: away from the attack (HitPlayer continues at line 2335). */
		State = STATE_HURT;
		/* PlayerObj.cs line 2295: the injury cues (male or female). */
		if (m_stats != nullptr && m_stats->hero.isFemale)
			Audio::PlaySound("Player_Female_Damage_03", "Player_Female_Damage_04", "Player_Female_Damage_05");
		else
			Audio::PlaySound("Player_Male_Injury_01", "Player_Male_Injury_02", "Player_Male_Injury_03");
		m_invincibleCounter = BaseInvincibilityTime;
		CurrentSpeed(0);
		if (AccelerationX() > 0 || HeadingX() > 0)
			AccelerationX(-KnockBack.X);
		else
			AccelerationX(KnockBack.X);
		AccelerationY(-KnockBack.Y);
	}
	else
	{
		/* TODO C++: death animation and the lineage handoff. */
		if (m_stats != nullptr && m_stats->hero.isFemale)
			Audio::PlaySound("Player_Female_Death_01", "Player_Female_Death_02");
		else
			Audio::PlaySound("Player_Male_Death_01", "Player_Male_Death_02");
		IsKilled = true;
		LockControls();
		Visible(false);
	}
}

void PlayerObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType)
{
	/* PlayerObj.cs line 2053: an enemy weapon touching the player's body
	 * hits (blocking comes with the Knight2 class). */
	if (thisBox->Type() == Consts::BODY_HITBOX && otherBox->Type() == Consts::WEAPON_HITBOX
	    && (collisionResponseType == Consts::COLLISIONRESPONSE_FIRSTBOXHIT
	        || collisionResponseType == Consts::COLLISIONRESPONSE_SECONDBOXHIT))
	{
		IPhysicsObj *otherParent = otherBox->PhysicsParent();
		if (otherParent != nullptr && otherParent->CollisionTypeTag() == 3 /* enemy */
		    && State != STATE_HURT)
		{
			/* PlayerObj.cs line 2065: the upgraded knight blocks
			 * projectiles while he has mana. */
			if (State == STATE_BLOCKING && (CurrentMana > 0 || m_blockInvincibleCounter > 0))
			{
				if (m_blockInvincibleCounter <= 0)
				{
					CurrentMana -= GameEV::KNIGHT_BLOCK_DRAIN;
					m_blockInvincibleCounter = BlockInvincibleTime;
					char buf[16];
					snprintf(buf, sizeof(buf), "-%d mp", GameEV::KNIGHT_BLOCK_DRAIN);
					if (m_levelScreen != nullptr)
						m_levelScreen->AddPopup(buf, X(), (float) Bounds().Top(),
						                        0xFF87CEEBu);
				}
				Audio::PlaySound("Player_Block");
				return;
			}
			if (m_invincibleCounter <= 0 && !ForceInvincible)
				HitPlayer(otherParent->DealsDamage(),
				          Vector2((float) otherBox->AbsRect().X, (float) otherBox->AbsRect().Y));
			return;
		}
	}

	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN)
		PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

void PlayerObj::UpdateAnimationState()
{
	/* PlayerObj.cs line 1502 (UpdateAnimationState). */
	switch (State)
	{
	case STATE_IDLE:
		if (SpriteName() != "PlayerIdle_Character")
			ChangeSprite("PlayerIdle_Character");
		if (!IsAnimating())
			PlayAnimation(true);
		break;
	case STATE_WALKING:
		if (SpriteName() != "PlayerWalking_Character")
			ChangeSprite("PlayerWalking_Character");
		if (!IsAnimating())
			PlayAnimation(true);
		break;
	case STATE_FLYING:
	case STATE_DRAGON:
	case STATE_JUMPING:
		if (AccelerationY() <= 0)
		{
			if (SpriteName() != "PlayerJumping_Character")
				ChangeSprite("PlayerJumping_Character");
		}
		else if (AccelerationY() > 0)
		{
			if (SpriteName() != "PlayerFalling_Character")
				ChangeSprite("PlayerFalling_Character");
		}
		if (!IsAnimating())
			PlayAnimation(true);
		break;
	case STATE_HURT:
		if (SpriteName() != "PlayerHurt_Character")
			ChangeSprite("PlayerHurt_Character");
		if (IsAnimating())
			StopAnimation();
		break;
	case STATE_DASHING:
		/* PlayerObj.cs line 1548: the dash sprite depends on the facing
		 * versus the dash direction (front dash = toward the camera). */
		if (HeadingX() < 0 && Flip() == SpriteEffects::None)
		{
			if (SpriteName() != "PlayerDash_Character")
				ChangeSprite("PlayerDash_Character");
		}
		else if (HeadingX() < 0 && Flip() == SpriteEffects::FlipHorizontally)
		{
			if (SpriteName() != "PlayerFrontDash_Character")
				ChangeSprite("PlayerFrontDash_Character");
		}
		else if (HeadingX() > 0 && Flip() == SpriteEffects::None)
		{
			if (SpriteName() != "PlayerFrontDash_Character")
				ChangeSprite("PlayerFrontDash_Character");
		}
		else if (HeadingX() > 0 && Flip() == SpriteEffects::FlipHorizontally)
		{
			if (SpriteName() != "PlayerDash_Character")
				ChangeSprite("PlayerDash_Character");
		}
		if (!IsAnimating())
			PlayAnimation(false);
		break;
	case STATE_BLOCKING:
		/* The default knight cannot block (CanBlock is Knight2 only). */
		if (SpriteName() != "PlayerBlock_Character")
		{
			ChangeSprite("PlayerBlock_Character");
			PlayAnimation(false);
		}
		break;
	default:
		break;
	}
}

void PlayerObj::CastSpell(bool activateSecondary, bool megaSpell)
{
	/* PlayerObj.cs line 2621 (CastSpell). The spell book UI, the spell
	 * icons and the impact effects are not ported yet. */
	if (m_stats == nullptr || m_levelScreen == nullptr)
		return;
	uint8_t spellType = m_stats->hero.spell;

	ProjectileData projData = SpellEV::GetProjData(spellType, this);
	projData.damage = (int) (TotalMagicDamage() * SpellEV::GetDamageMultiplier(spellType));

	int manaCost = (int) (SpellEV::GetManaCost(spellType)
	                       * (1 - m_stats->skills.ModifierAmount(Skills::SkillType::Mana_Cost_Down)));
	if (CurrentMana >= manaCost)
	{
		m_spellCastDelay = 0.5f;
		/* The Savant trait rerolls the spell every cast
		 * (PlayerObj.cs line 2644). */
		if (m_stats->hero.HasTrait(TraitType::Savant))
		{
			std::vector<uint8_t> spellList = Lineage::Generator::GetSpellList(m_stats->hero.classType);
			bool rerolled = false;
			while (!spellList.empty() && !rerolled)
			{
				uint8_t s = spellList[rand() % spellList.size()];
				if (s != SpellType::Translocator && s != SpellType::TimeStop
				    && s != SpellType::DamageShield && s != spellType)
				{
					m_stats->hero.spell = s;
					spellType = s;
					projData = SpellEV::GetProjData(spellType, this);
					projData.damage = (int) (TotalMagicDamage()
					                         * SpellEV::GetDamageMultiplier(spellType));
					rerolled = true;
				}
			}
		}
	}

	float altX = SpellEV::GetXValue(spellType);
	float altY = SpellEV::GetYValue(spellType);

	if (megaSpell)
	{
		/* The SpellSword's mega spells (PlayerObj.cs line 2655). */
		manaCost = (int) (manaCost * GameEV::SPELLSWORD_MANACOST_MOD);
		projData.scale = Vector2(projData.scale.X * GameEV::SPELLSWORD_SPELL_SCALE,
		                         projData.scale.Y * GameEV::SPELLSWORD_SPELL_SCALE);
		projData.damage = (int) (projData.damage * GameEV::SPELLSWORD_SPELLDAMAGE_MOD);
	}

	if (CurrentMana < manaCost)
		Audio::PlaySound("Error_Spell");
	else if (spellType != SpellType::Translocator && spellType != SpellType::Nuke
	         && !m_damageShieldCast && manaCost > 0)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "-%d mp", manaCost);
		m_levelScreen->AddPopup(buf, X(), (float) Bounds().Top(), 0xFF87CEEBu);
	}

	/* The Ambilevous trait mirrors the spell (PlayerObj.cs line 2669). */
	bool ambilevous = m_stats->hero.HasTrait(TraitType::Ambilevous);
	if (spellType != SpellType::Bounce && spellType != SpellType::DamageShield
	    && ambilevous)
		projData.sourceAnchor = Vector2(-projData.sourceAnchor.X, projData.sourceAnchor.Y);

	switch (spellType)
	{
	case SpellType::Dagger:
	case SpellType::Axe:
	case SpellType::TimeBomb:
	case SpellType::Close:
	case SpellType::DualBlades:
	case SpellType::DragonFire:
	case SpellType::DragonFireNeo:
		if (CurrentMana >= manaCost && !activateSecondary)
		{
			if (spellType == SpellType::DragonFireNeo)
				projData.lifespan = SpellEV::DRAGONFIRENEO_XVal;
			if (spellType == SpellType::Dagger)
				Audio::PlaySound("Cast_Dagger");
			else if (spellType == SpellType::Axe)
				Audio::PlaySound("Cast_Axe");
			else if (spellType == SpellType::DualBlades)
				Audio::PlaySound("Cast_Chakram");
			else if (spellType == SpellType::Close)
				Audio::PlaySound("Cast_GiantSword");
			else if (spellType == SpellType::DragonFire || spellType == SpellType::DragonFireNeo)
				Audio::PlaySound("Enemy_WallTurret_Fire_01", "Enemy_WallTurret_Fire_02");

			ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
			proj->AltX = altX;
			proj->AltY = altY;
			if (spellType == SpellType::Close)
			{
				/* The katana hangs in front of the player for
				 * CLOSE_XVal seconds. */
				proj->LifeSpan = altX;
				proj->Y(proj->Y() - 20);
			}
			if (spellType == SpellType::DualBlades)
			{
				/* A second blade on the mirrored diagonal. */
				projData.angle = Vector2(-10, -10);
				projData.sourceAnchor = ambilevous ? Vector2(-50, -30) : Vector2(50, -30);
				projData.rotationSpeed = -20;
				m_levelScreen->FireSpellProjectile(projData, spellType);
			}
			if (spellType == SpellType::TimeBomb)
			{
				proj->BlinkTime = altX / 1.5f;
				proj->LifeSpan = 20;
			}
			CurrentMana -= manaCost;
		}
		break;
	case SpellType::Boomerang:
		if (CurrentMana >= manaCost && !activateSecondary)
		{
			Audio::PlaySound("Cast_Boomerang");
			ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
			proj->IgnoreBoundsCheck = true;
			proj->AltX = altX;
			/* The cross turns around (PlayerObj.cs line 2749: the flip
			 * and Ambilevous decide the direction). */
			if ((Flip() == SpriteEffects::FlipHorizontally && !ambilevous)
			    || (Flip() == SpriteEffects::None && ambilevous))
				proj->AltX = -altX;
			proj->AltY = 0.5f;
			CurrentMana -= manaCost;
		}
		break;
	case SpellType::Bounce:
		if (CurrentMana >= manaCost && !activateSecondary)
		{
			Audio::PlaySound("Cast_Dagger");
			/* Four balls, each 90 degrees apart. */
			for (int i = 0; i < 4; i++)
			{
				ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
				proj->AltX = altX;
				proj->AltY = 0.5f;
				switch (i)
				{
				case 0: projData.sourceAnchor = Vector2(10, -10); break;
				case 1: projData.sourceAnchor = Vector2(10, 10); break;
				case 2: projData.sourceAnchor = Vector2(-10, 10); break;
				}
				projData.angle = Vector2(projData.angle.X + 90, projData.angle.Y + 90);
			}
			CurrentMana -= manaCost;
		}
		break;
	case SpellType::Nuke:
	{
		/* One crow per enemy of the room, up to 9
		 * (PlayerObj.cs line 2783). */
		if (m_levelScreen->CurrentRoom() == nullptr)
			break;
		int numEnemies = m_levelScreen->CurrentRoom()->ActiveEnemies();
		if (numEnemies > 9)
			numEnemies = 9;
		if (CurrentMana >= manaCost && !activateSecondary && numEnemies > 0)
		{
			Audio::PlaySound("Cast_Crowstorm");
			float angleStep = 360.0f / numEnemies;
			float startingAngle = 0;
			for (EnemyObj *enemy : m_levelScreen->CurrentRoom()->EnemyList())
			{
				if (enemy->NonKillable || enemy->IsKilled)
					continue;
				projData.target = enemy;
				ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
				proj->LifeSpan = 10;
				proj->AltX = 0.25f;   /* the wind-up before the chase */
				proj->AltY = 0.05f;
				proj->Orientation = startingAngle * (float) M_PI / 180.0f;
				proj->TurnSpeed = 0.075f;
				proj->IgnoreBoundsCheck = true;
				float rad = startingAngle * (float) M_PI / 180.0f;
				proj->Position(Vector2(X() + cosf(rad) * 200, Y() + sinf(rad) * 200));
				startingAngle += angleStep;
				if (startingAngle >= 360)
					break;
			}
			CurrentMana -= manaCost;
		}
		break;
	}
	case SpellType::DamageShield:
		if (m_damageShieldCast)
		{
			m_damageShieldCast = false;
			m_megaDamageShieldCast = false;
		}
		else if (CurrentMana >= manaCost && !activateSecondary)
		{
			m_damageShieldCast = true;
			if (megaSpell)
				m_megaDamageShieldCast = true;
			Audio::PlaySound("Cast_FireShield");
			/* The leaves spin around the player (altY of them, at
			 * distance 200) until the mana runs out. */
			float angleStep = 360.0f / altY;
			for (int i = 0; i < (int) altY; i++)
			{
				float angle = angleStep * i;
				projData.sourceAnchor = Vector2(0, 0);
				ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
				proj->LifeSpan = altX;
				proj->AltX = angle;
				proj->AltY = 200;
				proj->IgnoreBoundsCheck = true;
			}
			CurrentMana -= manaCost;
		}
		break;
	case SpellType::Laser:
		if (CurrentMana >= manaCost && !activateSecondary
		    && m_levelScreen->CurrentRoom() != nullptr)
		{
			ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, spellType);
			proj->AltX = 1;             /* the flash-in */
			proj->AltY = 0.5f;           /* the damage window */
			proj->Y(Y());
			float w = proj->Width() > 0 ? proj->Width() : 1;
			proj->X(m_levelScreen->CurrentRoom()->X() + w * 0.5f);
			proj->ScaleX(m_levelScreen->CurrentRoom()->Width() / w);
			proj->IgnoreBoundsCheck = true;
			CurrentMana -= manaCost;
		}
		break;
	case SpellType::TimeStop:
		if (m_timeStopCast)
		{
			m_levelScreen->StopTimeStop();
			m_timeStopCast = false;
		}
		else if (CurrentMana >= manaCost && !activateSecondary)
		{
			CurrentMana -= manaCost;
			m_levelScreen->CastTimeStop();
			m_timeStopCast = true;
		}
		break;
	case SpellType::Translocator:
		if (!m_translocatorActive && CurrentMana >= manaCost)
		{
			/* Drop the marker at the current position
			 * (PlayerObj.cs line 2917; the marker is the player's
			 * black silhouette in the C#, a plain spot here). */
			CurrentMana -= manaCost;
			m_translocatorActive = true;
			m_translocatorPos = Position();
			Audio::PlaySound("mfqt_teleport_out");
		}
		else if (m_translocatorActive)
		{
			Audio::PlaySound("mfqt_teleport_in");
			Vector2 target = m_translocatorPos;
			m_translocatorActive = false;
			Translocate(target);
		}
		break;
	case SpellType::Displacer:
		if (CurrentMana >= manaCost)
		{
			/* The displacer teleports the player to the first wall in
			 * the facing direction (RunDisplacerEffect,
			 * ProjectileObj.cs line 365; the port uses a straight
			 * ray, the rotated-terrain cases are not ported). */
			CurrentMana -= manaCost;
			float dir = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
			Translocate(Vector2(X() + dir * (ClosestWallDistance() - 20), Y()));
		}
		break;
	case SpellType::RapidDagger:
		if (CurrentMana >= manaCost)
		{
			CurrentMana -= manaCost;
			/* Five daggers in 0.2 seconds (ThrowDaggerProjectiles). */
			m_rapidDaggerQueue = 5;
			m_rapidDaggerTimer = 0;
		}
		break;
	default:
		break;
	}
}

void PlayerObj::CastDaggers(bool randomize)
{
	/* PlayerObj.cs line 3331 (CastDaggers): one rapid dagger, or one of
	 * the Traitor's queued axes (ThrowAxeProjectiles). */
	uint8_t spell = m_throwingAxes ? SpellType::Axe : SpellType::RapidDagger;
	ProjectileData projData = SpellEV::GetProjData(spell, this);
	if (m_throwingAxes)
		projData.damage = (int) (TotalMagicDamage()
		                         * PlayerEV::TRAITOR_AXE_DAMAGEMOD);
	else
	{
		projData.spriteName = "SpellDagger_Sprite";
		projData.angle = Vector2(0, 0);
		projData.sourceAnchor = Vector2(50, 0);
		projData.speed = 1750;
		projData.collidesWithTerrain = true;
		projData.destroysWithTerrain = true;
		projData.isCollidable = true;
		projData.scale = Vector2(2.5f, 2.5f);
		projData.damage = (int) (TotalMagicDamage() * SpellEV::RAPIDDAGGER_Damage);
	}
	if (randomize)
		projData.angleOffset = (float) (rand() % 17 - 8);
	m_levelScreen->FireSpellProjectile(projData, spell);
	Audio::PlaySound("Cast_Dagger");
}

void PlayerObj::Translocate(Vector2 position)
{
	/* PlayerObj.cs line 2978 (Translocate); the scale tween of the C#
	 * becomes an instant move (the camera snaps with UpdateCamera). */
	Position(position);
	Acceleration(Vector2(0, 0));
}

float PlayerObj::ClosestWallDistance() const
{
	/* ProjectileObj.cs line 365 (RunDisplacerEffect): the first wall in
	 * the facing direction (a straight ray; the rotated-terrain cases
	 * of the C# are not ported). */
	if (m_levelScreen == nullptr || m_levelScreen->CurrentRoom() == nullptr)
		return 6600;
	float dir = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	float closest = 6600;
	Rectangle pb = Bounds();
	for (const TerrainObj *terrain : m_levelScreen->CurrentRoom()->TerrainObjList())
	{
		Rectangle tb = terrain->TerrainBounds();
		if (tb.Top() < pb.Bottom() && tb.Bottom() > pb.Top())
		{
			float distance = dir > 0 ? tb.Left() - pb.Right()
			                 : pb.Left() - tb.Right();
			if (distance >= 0 && distance < closest)
				closest = distance;
		}
	}
	return closest;
}

void PlayerObj::CastClassPower()
{
	/* PlayerObj.cs lines 1011-1050: the special of the upgraded classes
	 * and the Traitor on the block button. */
	if (m_stats == nullptr || m_levelScreen == nullptr)
		return;
	uint8_t classType = m_stats->hero.classType;
	if (classType == ClassType::SpellSword2)
	{
		if (m_spellCastDelay <= 0)
			CastSpell(false, true);
	}
	else if (classType == ClassType::Lich2)
		ConvertHPtoMP();
	else if (classType == ClassType::Assassin2 && CurrentMana > 0)
	{
		if (!m_assassinSpecialActive)
			ActivateAssassinAbility();
		else
			DisableAssassinAbility();
	}
	else if (classType == ClassType::Wizard2)
		SwapSpells();
	else if (classType == ClassType::Ninja2)
		NinjaTeleport();
	else if (classType == ClassType::Barbarian2)
		CastFuhRohDah();
	else if (classType == ClassType::Traitor)
	{
		if (CurrentMana >= PlayerEV::TRAITOR_AXE_MANACOST && m_spellCastDelay <= 0)
		{
			CurrentMana -= PlayerEV::TRAITOR_AXE_MANACOST;
			m_spellCastDelay = 0.5f;
			ThrowAxeProjectiles();
		}
	}
}

void PlayerObj::ConvertHPtoMP()
{
	/* PlayerObj.cs line 3000: half the health becomes max mana. */
	if (m_stats == nullptr || MaxHealth() <= 1)
		return;
	int baseHealthLost = (int) ((MaxHealth() - m_stats->LichHealth)
	                             * GameEV::LICH_HEALTH_CONVERSION_PERCENT);
	int lichHealthLost = (int) (m_stats->LichHealth
	                             * GameEV::LICH_HEALTH_CONVERSION_PERCENT);
	if (MaxMana() + baseHealthLost + lichHealthLost < m_stats->LichMana)
	{
		Audio::PlaySound("Lich_Swap");
		m_stats->LichHealthMod *= GameEV::LICH_HEALTH_CONVERSION_PERCENT;
		m_stats->LichHealth -= lichHealthLost;
		m_stats->LichMana += baseHealthLost + lichHealthLost;
		CurrentMana += baseHealthLost + lichHealthLost;
		if (CurrentHealth > MaxHealth())
			CurrentHealth = MaxHealth();
		char buf[32];
		snprintf(buf, sizeof(buf), "-%d hp", baseHealthLost + lichHealthLost);
		m_levelScreen->AddPopup(buf, X(), (float) Bounds().Top() - 60, 0xFFFF0000u);
		snprintf(buf, sizeof(buf), "+%d mp", baseHealthLost + lichHealthLost);
		m_levelScreen->AddPopup(buf, X(), (float) Bounds().Top() - 30, 0xFF4169E1u);
	}
	else
		Audio::PlaySound("Error_Spell");
}

void PlayerObj::ActivateAssassinAbility()
{
	/* PlayerObj.cs line 3035: the stealth costs 5 mp and drains while
	 * up; the hero cannot be hit (ForceInvincible). */
	if (CurrentMana >= GameEV::ASSASSIN_ACTIVE_INITIAL_COST)
	{
		Audio::PlaySound("Assassin_Stealth_Enter");
		CurrentMana -= GameEV::ASSASSIN_ACTIVE_INITIAL_COST;
		Opacity(0.05f);   /* the C# tweens it */
		m_assassinSpecialActive = true;
		ForceInvincible = true;
	}
}

void PlayerObj::DisableAssassinAbility()
{
	/* PlayerObj.cs line 3048. */
	Audio::PlaySound("Assassin_Stealth_Exit");
	Opacity(1);
	m_assassinSpecialActive = false;
	ForceInvincible = false;
}

void PlayerObj::SwapSpells()
{
	/* PlayerObj.cs line 3056: the upgraded Wizard cycles his spell list. */
	if (m_stats == nullptr)
		return;
	Audio::PlaySound("Spell_Switch");
	std::vector<uint8_t> spells = Lineage::Generator::GetSpellList(ClassType::Wizard2);
	for (size_t i = 0; i < spells.size(); i++)
	{
		if (spells[i] == m_stats->hero.spell)
		{
			m_stats->hero.spell = spells[(i + 1) % spells.size()];
			return;
		}
	}
	if (!spells.empty())
		m_stats->hero.spell = spells[0];
}

void PlayerObj::NinjaTeleport()
{
	/* PlayerObj.cs line 3086: a short teleport toward the facing wall. */
	if (CurrentMana < GameEV::NINJA_TELEPORT_COST || m_ninjaTeleportDelay > 0)
		return;
	CurrentMana -= GameEV::NINJA_TELEPORT_COST;
	m_ninjaTeleportDelay = 0.5f;
	Audio::PlaySound("Ninja_Teleport");
	float distance = ClosestWallDistance();
	float move = GameEV::NINJA_TELEPORT_DISTANCE;
	if (distance < move)
		move = distance - TerrainBounds().Width * 0.5f;
	float dir = Flip() == SpriteEffects::FlipHorizontally ? -1.0f : 1.0f;
	X(X() + dir * move);
}

void PlayerObj::CastFuhRohDah()
{
	/* PlayerObj.cs line 3229 (CastFuhRohDah): the Barbarian shout, a
	 * wide projectile that knocks the enemies back without damage. */
	if (CurrentMana < GameEV::BARBARIAN_SHOUT_INITIAL_COST)
		return;
	CurrentMana -= GameEV::BARBARIAN_SHOUT_INITIAL_COST;
	Audio::PlaySound("Cast_FasRoDus");
	ProjectileData projData;
	projData.spriteName = "SpellShout_Sprite";
	projData.source = this;
	projData.angle = Vector2(0, 0);
	projData.sourceAnchor = Vector2(60, -20);
	projData.isWeighted = false;
	projData.lifespan = 2;
	projData.scale = Vector2(1, 1);
	projData.damage = 0;
	projData.speed = 0;
	projData.lockPosition = true;
	ProjectileObj *proj = m_levelScreen->FireSpellProjectile(projData, SpellType::Shout);
	proj->IgnoreBoundsCheck = true;
}

void PlayerObj::ThrowAxeProjectiles()
{
	/* The Traitor's axes: three axe throws like the rapid daggers
	 * (ThrowAxeProjectiles, the C# Tweener calls). */
	m_throwingAxes = true;
	m_rapidDaggerQueue = 3;
	m_rapidDaggerTimer = 0;
}

} // namespace RogueCastle
