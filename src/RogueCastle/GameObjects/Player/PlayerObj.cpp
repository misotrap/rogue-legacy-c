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

	/* PlayerObj.cs line 397. */
	Scale(Vector2(2, 2));
	ForceDraw(true);

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

void PlayerObj::Update(GameTime *gameTime)
{
	float elapsedSeconds = (float) gameTime->ElapsedGameTime;

	/* PlayerObj.cs line 1254 (Update). */
	if (m_dropThroughGroundTimer > 0)
		m_dropThroughGroundTimer -= elapsedSeconds;

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

	/* PlayerObj.cs line 1335: friction on knockback, then the position. */
	if (AccelerationX() < 0)
		AccelerationX(AccelerationX() + 200.0f * elapsedSeconds);
	X(X() + Heading().X * (CurrentSpeed() * elapsedSeconds));

	UpdatePlayerCollision();
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
	default:
		break;
	}
}

} // namespace RogueCastle
