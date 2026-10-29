/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Platform.cs. */
#include "EnemyObj_Platform.h"
#include "../../Types/EnemyType.h"
#include "../../Types/TraitType.h"
#include "../Player/PlayerObj.h"
#include "../../../Audio/Audio.h"
#include "../../../debugflags.h"
#include <cstdio>

namespace RogueCastle
{

EnemyObj_Platform::EnemyObj_Platform(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyPlatform_Character", target, difficulty)
{
	Type = EnemyType::Platform;
	NonKillable = true;
	LockFlip = true;
	CollisionTypeTag(1);   // CollisionType_WALL
	StopAnimation();
}

void EnemyObj_Platform::InitializeEV()
{
	Scale(Vector2(2, 2));
	AnimationDelay(1 / 30.0f);
	Speed = 0;
	MaxHealth = 999;
	CurrentHealth = MaxHealth;
	EngageRadius = 30;
	ProjectileRadius = 20;
	MeleeRadius = 10;
	CooldownTime = 2;
	KnockBack = Vector2(1, 2);
	Damage = 25;
	XPValue = 2;
	AlwaysFaceTarget = false;
	CanFallOffLedges = false;
	CanBeKnockedBack = false;
	IsWeightedFlag = false;
	if (PlatformsOpen())
	{
		m_isExtended = true;
		PlayAnimation("EndRetract", "EndRetract");
	}
}

bool EnemyObj_Platform::PlatformsOpen() const
{
	return m_target != nullptr && m_target->Stats() != nullptr
	    && m_target->Stats()->hero.HasTrait(TraitType::PlatformsOpen);
}

void EnemyObj_Platform::UpdateFloor(float dt)
{
	// The plate's top, once fully out (5 frames at 30 per second)
	m_extendTime = m_isExtended ? m_extendTime + dt : 0;
	m_floor->Enabled = m_isExtended && (m_extendTime >= 0.17f || PlatformsOpen());
	if (m_floor->Enabled && NumChildren() > 0)
	{
		// The plate's bounds are relative to the platform
		Rectangle plate = GetChildAt(0)->Bounds();
		Rectangle b((int) X() + plate.X, (int) Y() + plate.Y, plate.Width, plate.Height);
		Rectangle now = m_floor->TerrainBounds();
		if (now.X != b.X || now.Y != b.Y || now.Width != b.Width)
			m_floor->Populate(b.X, b.Y, b.Width, 20, true, false, false, false);
	}
}

void EnemyObj_Platform::UpdateEnemy(GameTime *gameTime)
{
	float dt = (float) gameTime->ElapsedGameTime;
	if (PlatformsOpen())
	{
		if (!m_isExtended)
		{
			m_isExtended = true;
			PlayAnimation("EndRetract", "EndRetract");
		}
		UpdateFloor(dt);
		return;
	}
	if (m_foldTimer > 0) m_foldTimer -= dt;
	if (m_warningBlinks > 0 && (m_warningTimer -= dt) <= 0)
	{
		Blink(Color(255, 0, 0), 0.05f);
		m_warningBlinks--;
		m_warningTimer = 0.1f;
	}
	if (m_retractCounter > 0)
	{
		m_retractCounter -= dt;
		if (m_retractCounter <= 1.5f && !m_blinkedWarning)
		{
			m_blinkedWarning = true;
			m_warningBlinks = 10;
			m_warningTimer = 0;
		}
		if (m_retractCounter <= 0)
		{
			m_isExtended = false;
			m_foldTimer = 0.35f;   // the folding animation
			PlayAnimation("StartExtract", "EndExtract");   // the names are flipped in the sheet
			Audio::PlaySound("Platform_Activate_03", "Platform_Activate_04");
		}
	}
	UpdateFloor(dt);
}

void EnemyObj_Platform::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	(void) damage; (void) collisionPt; (void) isPlayer;
	if (rl_trace())
		fprintf(stderr, "platform hit: air %d extended %d frame %d/%d\n", m_target != nullptr && m_target->IsAirAttacking,
		        (int) m_isExtended, CurrentFrame(), TotalFrames());
	// Only a down strike unfolds it, from fully folded
	if (m_target == nullptr || !m_target->IsAirAttacking)
		return;
	if (m_isExtended || m_foldTimer > 0)   // folded back completely first
		return;
	Audio::PlaySound("EnemyHit1", "EnemyHit2", "EnemyHit3");
	Blink(Color(255, 0, 0), 0.1f);
	m_isExtended = true;
	m_blinkedWarning = false;
	PlayAnimation("StartRetract", "EndRetract");
	Audio::PlaySound("Platform_Activate_01", "Platform_Activate_02");
	m_retractCounter = 3;   // RetractDelay
	// The down strike bounces off it
	m_target->IsAirAttacking = false;
	m_target->AccelerationY(-m_target->AirAttackKnockBack);
	m_target->NumAirBounces++;
}

void EnemyObj_Platform::ResetState()
{
	if (PlatformsOpen())
		return;
	PlayAnimation(1, 1);
	m_isExtended = false;
	m_blinkedWarning = false;
	m_retractCounter = 0;
	m_warningBlinks = 0;
	m_foldTimer = 0;
	m_floor->Enabled = false;
	EnemyObj::ResetState();
	StopAnimation();
}

} // namespace RogueCastle
