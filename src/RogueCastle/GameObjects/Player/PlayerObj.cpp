/* Port of RogueCastle/src/Game Objects/Player/PlayerObj.cs. */
#include "PlayerObj.h"
#include "../../Game.h"
#include "../../../DS2DEngine/Consts.h"

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
			}
			else if ((Game::GlobalInput.JustPressed(PLAYER_JUMP1) || Game::GlobalInput.JustPressed(PLAYER_JUMP2))
			         && !m_isTouchingGround && m_doubleJumpCount < TotalDoubleJumps)
			{
				State = STATE_JUMPING;
				AccelerationY(-DoubleJumpHeight);
				m_isJumping = true;
				m_doubleJumpCount++;
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
		    && State != STATE_HURT && m_invincibleCounter <= 0)
		{
			if (m_invincibleCounter <= 0)
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

} // namespace RogueCastle
