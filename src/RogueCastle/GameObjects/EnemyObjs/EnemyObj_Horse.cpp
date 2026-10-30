/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Horse.cs. */
#include "EnemyObj_Horse.h"
#include "../../EVs/EnemyEV.h"
#include "../../Types/EnemyType.h"
#include "../Player/PlayerObj.h"
#include "../TerrainObj.h"
#include "../ProjectileObj.h"
#include "../../Types/SpellType.h"
#include "../../../Audio/Audio.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include "../../../DS2DEngine/CollisionMath.h"

namespace RogueCastle
{

using namespace DS2DEngine;

EnemyObj_Horse::EnemyObj_Horse(PlayerObj *target, EnemyDifficulty difficulty)
	: EnemyObj("EnemyHorseRun_Character", target, difficulty)
{
	Type = EnemyType::Horse;
	Heading(Vector2(1, 0));
}

/* The difficulty blocks of InitializeEV only differ by their EV names
 * (EnemyObj_Horse.cs line 26), so they collapse into one. */
void EnemyObj_Horse::InitializeOne(EnemyDifficulty difficulty)
{
	switch (difficulty)
	{
	case MINIBOSS:
		Name = EnemyEV::Horse_Miniboss_Name;
		MaxHealth = EnemyEV::Horse_Miniboss_MaxHealth;
		Damage = EnemyEV::Horse_Miniboss_Damage;
		XPValue = EnemyEV::Horse_Miniboss_XPValue;
		MinMoneyDropAmount = EnemyEV::Horse_Miniboss_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Horse_Miniboss_MaxDropAmount;
		MoneyDropChance = EnemyEV::Horse_Miniboss_DropChance;
		Speed = EnemyEV::Horse_Miniboss_Speed;
		TurnSpeed = EnemyEV::Horse_Miniboss_TurnSpeed;
		ProjectileSpeed = EnemyEV::Horse_Miniboss_ProjectileSpeed;
		JumpHeight = EnemyEV::Horse_Miniboss_Jump;
		CooldownTime = EnemyEV::Horse_Miniboss_Cooldown;
		AnimationDelay(1 / EnemyEV::Horse_Miniboss_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Horse_Miniboss_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Horse_Miniboss_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Horse_Miniboss_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Horse_Miniboss_IsWeighted;
		Scale(Vector2(EnemyEV::Horse_Miniboss_Scale.X, EnemyEV::Horse_Miniboss_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Horse_Miniboss_ProjectileScale.X, EnemyEV::Horse_Miniboss_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Horse_Miniboss_Tint.R, EnemyEV::Horse_Miniboss_Tint.G, EnemyEV::Horse_Miniboss_Tint.B));
		MeleeRadius = EnemyEV::Horse_Miniboss_MeleeRadius;
		ProjectileRadius = EnemyEV::Horse_Miniboss_ProjectileRadius;
		EngageRadius = EnemyEV::Horse_Miniboss_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Horse_Miniboss_KnockBack.X, EnemyEV::Horse_Miniboss_KnockBack.Y);
		break;
	case EXPERT:
		Name = EnemyEV::Horse_Expert_Name;
		MaxHealth = EnemyEV::Horse_Expert_MaxHealth;
		Damage = EnemyEV::Horse_Expert_Damage;
		XPValue = EnemyEV::Horse_Expert_XPValue;
		MinMoneyDropAmount = EnemyEV::Horse_Expert_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Horse_Expert_MaxDropAmount;
		MoneyDropChance = EnemyEV::Horse_Expert_DropChance;
		Speed = EnemyEV::Horse_Expert_Speed;
		TurnSpeed = EnemyEV::Horse_Expert_TurnSpeed;
		ProjectileSpeed = EnemyEV::Horse_Expert_ProjectileSpeed;
		JumpHeight = EnemyEV::Horse_Expert_Jump;
		CooldownTime = EnemyEV::Horse_Expert_Cooldown;
		AnimationDelay(1 / EnemyEV::Horse_Expert_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Horse_Expert_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Horse_Expert_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Horse_Expert_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Horse_Expert_IsWeighted;
		Scale(Vector2(EnemyEV::Horse_Expert_Scale.X, EnemyEV::Horse_Expert_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Horse_Expert_ProjectileScale.X, EnemyEV::Horse_Expert_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Horse_Expert_Tint.R, EnemyEV::Horse_Expert_Tint.G, EnemyEV::Horse_Expert_Tint.B));
		MeleeRadius = EnemyEV::Horse_Expert_MeleeRadius;
		ProjectileRadius = EnemyEV::Horse_Expert_ProjectileRadius;
		EngageRadius = EnemyEV::Horse_Expert_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Horse_Expert_KnockBack.X, EnemyEV::Horse_Expert_KnockBack.Y);
		break;
	case ADVANCED:
		Name = EnemyEV::Horse_Advanced_Name;
		MaxHealth = EnemyEV::Horse_Advanced_MaxHealth;
		Damage = EnemyEV::Horse_Advanced_Damage;
		XPValue = EnemyEV::Horse_Advanced_XPValue;
		MinMoneyDropAmount = EnemyEV::Horse_Advanced_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Horse_Advanced_MaxDropAmount;
		MoneyDropChance = EnemyEV::Horse_Advanced_DropChance;
		Speed = EnemyEV::Horse_Advanced_Speed;
		TurnSpeed = EnemyEV::Horse_Advanced_TurnSpeed;
		ProjectileSpeed = EnemyEV::Horse_Advanced_ProjectileSpeed;
		JumpHeight = EnemyEV::Horse_Advanced_Jump;
		CooldownTime = EnemyEV::Horse_Advanced_Cooldown;
		AnimationDelay(1 / EnemyEV::Horse_Advanced_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Horse_Advanced_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Horse_Advanced_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Horse_Advanced_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Horse_Advanced_IsWeighted;
		Scale(Vector2(EnemyEV::Horse_Advanced_Scale.X, EnemyEV::Horse_Advanced_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Horse_Advanced_ProjectileScale.X, EnemyEV::Horse_Advanced_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Horse_Advanced_Tint.R, EnemyEV::Horse_Advanced_Tint.G, EnemyEV::Horse_Advanced_Tint.B));
		MeleeRadius = EnemyEV::Horse_Advanced_MeleeRadius;
		ProjectileRadius = EnemyEV::Horse_Advanced_ProjectileRadius;
		EngageRadius = EnemyEV::Horse_Advanced_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Horse_Advanced_KnockBack.X, EnemyEV::Horse_Advanced_KnockBack.Y);
		break;
	default: /* BASIC */
		Name = EnemyEV::Horse_Basic_Name;
		MaxHealth = EnemyEV::Horse_Basic_MaxHealth;
		Damage = EnemyEV::Horse_Basic_Damage;
		XPValue = EnemyEV::Horse_Basic_XPValue;
		MinMoneyDropAmount = EnemyEV::Horse_Basic_MinDropAmount;
		MaxMoneyDropAmount = EnemyEV::Horse_Basic_MaxDropAmount;
		MoneyDropChance = EnemyEV::Horse_Basic_DropChance;
		Speed = EnemyEV::Horse_Basic_Speed;
		TurnSpeed = EnemyEV::Horse_Basic_TurnSpeed;
		ProjectileSpeed = EnemyEV::Horse_Basic_ProjectileSpeed;
		JumpHeight = EnemyEV::Horse_Basic_Jump;
		CooldownTime = EnemyEV::Horse_Basic_Cooldown;
		AnimationDelay(1 / EnemyEV::Horse_Basic_AnimationDelay);
		AlwaysFaceTarget = EnemyEV::Horse_Basic_AlwaysFaceTarget;
		CanFallOffLedges = EnemyEV::Horse_Basic_CanFallOffLedges;
		CanBeKnockedBack = EnemyEV::Horse_Basic_CanBeKnockedBack;
		IsWeightedFlag = EnemyEV::Horse_Basic_IsWeighted;
		Scale(Vector2(EnemyEV::Horse_Basic_Scale.X, EnemyEV::Horse_Basic_Scale.Y));
		ProjectileScale = Vector2(EnemyEV::Horse_Basic_ProjectileScale.X, EnemyEV::Horse_Basic_ProjectileScale.Y);
		if (NumChildren() > 0)
			GetChildAt(0)->TextureColor(Color(EnemyEV::Horse_Basic_Tint.R, EnemyEV::Horse_Basic_Tint.G, EnemyEV::Horse_Basic_Tint.B));
		MeleeRadius = EnemyEV::Horse_Basic_MeleeRadius;
		ProjectileRadius = EnemyEV::Horse_Basic_ProjectileRadius;
		EngageRadius = EnemyEV::Horse_Basic_EngageRadius;
		ProjectileDamage = Damage;
		KnockBack = Vector2(EnemyEV::Horse_Basic_KnockBack.X, EnemyEV::Horse_Basic_KnockBack.Y);
		break;
	}
	CurrentSpeed(Speed);
}

void EnemyObj_Horse::InitializeEV()
{
	InitializeOne(DifficultyType);
}

void EnemyObj_Horse::DropFireProjectile()
{
	/* DropFireProjectile (line 450): a fire puddle at the feet, alive
	 * for m_fireDropLifespan. */
	UpdateCollisionBoxes();
	float anchorY = (float) (Bounds().Bottom() - (int) Y()) - 10;
	m_levelScreen->FireProjectile("SpellDamageShield_Sprite",
	                              Position() + Vector2(0, anchorY), Vector2(1, 0),
	                              0, (int) ProjectileDamage, ProjectileScale,
	                              false, this, m_fireDropLifespan);
}

void EnemyObj_Horse::CastFireShield(int numFires)
{
	/* CastFireShield (line 474): numFires flames orbit the horse at
	 * m_fireDistance, m_fireRotationSpeed degrees per second, for its
	 * whole life (Lifespan 999999, DestroysWithEnemy false) - the same
	 * DamageShield spell as the player's leaves, with the horse as
	 * Target (ProjectileObj.cs line 189: Source is EnemyObj). */
	if (m_levelScreen == nullptr)
		return;
	Audio::PlaySound("Cast_FireShield");
	for (int i = 0; i < numFires; i++)
	{
		float angle = (360.0f / numFires) * i;
		ProjectileObj *proj = m_levelScreen->FireProjectile("SpellDamageShield_Sprite",
			Position(), Vector2(1, 0), m_fireRotationSpeed,
			(int) ProjectileDamage, Vector2(m_fireShieldScale, m_fireShieldScale),
			false, this, 999999);
		if (proj == nullptr) continue;
		proj->Spell = SpellType::DamageShield;
		proj->AltX = angle;   /* the orbit's angle and distance */
		proj->AltY = m_fireDistance;
		proj->Target = this;
		proj->DestroysWithEnemy = false;
		proj->IgnoreBoundsCheck = true;
		m_fireShieldList.push_back(proj);
	}
}

void EnemyObj_Horse::TurnHorse()
{
	/* TurnHorse: the turn animation while keeping the old direction,
	 * then the flip. */
	m_turning = true;
	ChangeSprite("EnemyHorseTurn_Character");
	PlayAnimation(true);
	m_actionTimer = 0.25f;
}

void EnemyObj_Horse::UpdateEnemy(GameTime *gameTime)
{
	float elapsed = (float) gameTime->ElapsedGameTime;

	/* Advanced+ drop fire while galloping (line 392). */
	if (DifficultyType == ADVANCED || DifficultyType == EXPERT)
	{
		if (m_fireDropTimer > 0)
		{
			m_fireDropTimer -= elapsed;
			if (m_fireDropTimer <= 0)
			{
				DropFireProjectile();
				m_fireDropTimer = m_fireDropInterval;
			}
		}
	}
	/* The expert keeps two flames orbiting (line 405). */
	if (DifficultyType == EXPERT && !IsPaused)
	{
		for (size_t i = 0; i < m_fireShieldList.size(); )
			if (m_fireShieldList[i]->IsKilled)
				m_fireShieldList.erase(m_fireShieldList.begin() + i);
			else
				i++;
		if (m_fireShieldList.empty())
			CastFireShield(m_numFireShieldObjs);
	}

	if (m_actionTimer > 0)
		m_actionTimer -= elapsed;

	if (m_turning)
	{
		/* The turn keeps the old heading (MoveDirection before the flip
		 * in turnRightLS/turnLeftLS, lines 168, 174). */
		if (m_actionTimer <= 0)
		{
			m_turning = false;
			HeadingX(-HeadingX());
			Flip(Flip() == SpriteEffects::FlipHorizontally ? SpriteEffects::None
			                                              : SpriteEffects::FlipHorizontally);
			ChangeSprite("EnemyHorseRun_Character");
			PlayAnimation(true);
			CurrentSpeed(Speed);
		}
		return;
	}

	/* Run to the room bounds or turn at the ledges (line 405): probes
	 * ahead of the feet like the original. */
	if (m_collisionCheckTimer > 0)
		m_collisionCheckTimer -= elapsed;
	if (m_levelScreen == nullptr || m_levelScreen->CurrentRoom() == nullptr)
		return;
	Rectangle rb = m_levelScreen->CurrentRoom()->Bounds();
	bool atBounds = Bounds().Left() < rb.Left() || Bounds().Right() > rb.Right();
	bool turn = true;
	Rectangle b = Bounds();
	Rectangle collPt, collPt2;
	if (Flip() == SpriteEffects::FlipHorizontally)
	{
		collPt = Rectangle(b.X - 10, b.Bottom() + 20, 5, 5);
		collPt2 = Rectangle(b.Right() + 50, b.Bottom() - 20, 5, 5);
	}
	else
	{
		collPt = Rectangle(b.Right() + 10, b.Bottom() + 20, 5, 5);
		collPt2 = Rectangle(b.X - 50, b.Bottom() - 20, 5, 5);
	}
	for (TerrainObj *terrain : m_levelScreen->CurrentRoom()->TerrainObjList())
	{
		if (CollisionMath::Intersects(terrain->TerrainBounds(), collPt)
		    || CollisionMath::Intersects(terrain->TerrainBounds(), collPt2))
		{
			turn = false;
			break;
		}
	}
	if ((atBounds || turn) && m_collisionCheckTimer <= 0)
	{
		m_collisionCheckTimer = 0.5f;
		TurnHorse();
	}
	else
		CurrentSpeed(Speed);
}

void EnemyObj_Horse::RunBasicLogic()
{

	/* EnemyObj_Horse.cs line 227: run in the facing direction. */
	if (!m_actionActive)
	{
		m_actionActive = true;
		ChangeSprite("EnemyHorseRun_Character");
		PlayAnimation(true);
		/* The gallop direction follows the flip (ResetState zeroes the
		 * heading, so it is re-derived here). */
		HeadingX(Flip() == SpriteEffects::FlipHorizontally ? -1 : 1);
		CurrentSpeed(Speed);
	}
}

} // namespace RogueCastle
