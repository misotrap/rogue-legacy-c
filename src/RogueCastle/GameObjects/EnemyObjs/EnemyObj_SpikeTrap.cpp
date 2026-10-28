/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SpikeTrap.cs. */
#include "EnemyObj_SpikeTrap.h"
#include "../../Types/EnemyType.h"
#include "../../Types/TraitType.h"
#include "../Player/PlayerObj.h"
#include "../../../DS2DEngine/CollisionMath.h"

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_SpikeTrap::EnemyObj_SpikeTrap(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemySpikeTrap_Character", target, difficulty)
{
	/* EnemyObj_SpikeTrap.cs line 148 (constructor). */
	Type = EnemyType::SpikeTrap;
	StopAnimation();
	NonKillable = true;
}

void EnemyObj_SpikeTrap::InitializeEV()
{
	/* EnemyObj_SpikeTrap.cs line 17: the same stats for every difficulty. */
	Scale(Vector2(2.0f, 2.0f));
	AnimationDelay(1 / 10.0f);
	Speed = 0;
	MaxHealth = 10;
	EngageRadius = 2100;
	ProjectileRadius = 2200;
	MeleeRadius = 650;
	CooldownTime = 2.0f;
	KnockBack = Vector2(1, 2);
	Damage = 25;
	JumpHeight = 20.5f;
	AlwaysFaceTarget = false;
	CanFallOffLedges = false;
	XPValue = 2;
	CanBeKnockedBack = false;
	IsWeightedFlag = false;
	Name = "Spike Trap";
	IsCollidableFlag = false;
	PlayAnimation(1, 1, false);   /* Reset(): the hidden frame */
}

void EnemyObj_SpikeTrap::UpdateEnemy(GameTime *gameTime)
{
	(void) gameTime;
	/* EnemyObj_SpikeTrap.cs line 106 (Update): the detection rect sits on
	 * top of the trap (X - w/2, Y - h, w, h). */
	Rectangle detect((int) (X() - m_detectWidth / 2.0f),
	                 (int) (Y() - m_detectHeight), m_detectWidth, m_detectHeight);
	bool playerOn = CollisionMath::Intersects(detect, m_target->Bounds())
		&& (m_target->Stats() == nullptr || !m_target->Stats()->hero.HasTrait(TraitType::PAD));   // PAD: not felt

	switch (m_trapState)
	{
	case TRAP_HIDDEN:
		if (playerOn)
		{
			/* Extract (line 116): collidable, frames 1-2. */
			m_trapState = TRAP_EXTRACTING;
			IsCollidableFlag = true;
			m_actionPhase = 0;
			PlayAnimation(1, 2, false);
		}
		break;
	case TRAP_EXTRACTING:
		if (m_actionPhase == 0 && !IsAnimating())
		{
			/* DelayLogicAction(ExtractDelay): the tell. */
			m_actionPhase = 1;
			m_actionTimer = ExtractDelay;
		}
		else if (m_actionPhase == 1 && m_actionTimer <= 0)
		{
			m_actionPhase = 2;
			PlayAnimation(2, 4, false);
		}
		else if (m_actionPhase == 2 && !IsAnimating())
			m_trapState = TRAP_EXTRACTED;
		break;
	case TRAP_EXTRACTED:
		if (!playerOn)
		{
			/* Retract (line 125): not collidable any more, frames 5-7. */
			m_trapState = TRAP_RETRACTING;
			IsCollidableFlag = false;
			PlayAnimation(5, 7, false);
		}
		break;
	case TRAP_RETRACTING:
		if (!IsAnimating())
		{
			m_trapState = TRAP_HIDDEN;
			PlayAnimation(1, 1, false);
		}
		break;
	}
}

void EnemyObj_SpikeTrap::ResetState()
{
	/* EnemyObj_SpikeTrap.cs line 154 (ResetState): back to the hidden
	 * frame and harmless. */
	PlayAnimation(1, 1, false);
	m_trapState = TRAP_HIDDEN;
	IsCollidableFlag = false;
	EnemyObj::ResetState();
	/* The base restarts the looping animation; the trap stays on frame 1. */
	StopAnimation();
	PlayAnimation(1, 1, false);
}

} // namespace RogueCastle
