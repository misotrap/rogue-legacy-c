/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj.cs (subset). */
#include "EnemyObj.h"
#include "../../EVs/GameEV.h"
#include "../../Types/TraitType.h"
#include "../../../DS2DEngine/CollisionMath.h"
#include "../../../DS2DEngine/Consts.h"
#include "../Player/PlayerObj.h"
#include "../BreakableObj.h"
#include "../ItemDropObj.h"
#include "../../EVs/GameEV.h"
#include "../../Types/EnemyType.h"
#include "../../Types/SpellType.h"
#include "../../Screens/ProceduralLevelScreen.h"
#include <cmath>
#include "../../../Audio/Audio.h"
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

static float DistanceBetweenPts(Vector2 a, Vector2 b)
{
	float dx = a.X - b.X, dy = a.Y - b.Y;
	return sqrtf(dx * dx + dy * dy);
}

EnemyObj::EnemyObj(const std::string &spriteName, PlayerObj *target, EnemyDifficulty difficulty)
	: PhysicsObjContainer(spriteName)
{
	/* EnemyObj.cs line 163 (constructor). */
	CollisionTypeTag(3);   /* GameTypes.CollisionType_ENEMY */
	m_target = target;
	DifficultyType = difficulty;
	m_resetSpriteName = spriteName;
	PlayAnimation(true);
	IsWeightedFlag = true;
	IsCollidableFlag = true;
	Scale(Vector2(2, 2));   /* most enemies draw at scale ~2-2.5 (per-EV) */
}

void EnemyObj::InitializeBaseEV()
{
	/* EnemyObj.cs line 116. */
	Speed = 1;
	MaxHealth = 10;
	EngageRadius = 400;
	ProjectileRadius = 200;
	MeleeRadius = 50;
	KnockBack = Vector2(0, 0);
	Damage = 5;
	ProjectileScale = Vector2(1, 1);
	XPValue = 0;
	ProjectileDamage = 5;
	MinMoneyDropAmount = 1;
	MaxMoneyDropAmount = 1;
	MoneyDropChance = 0.50f;
	ForceDraw(true);
}

void EnemyObj::Initialize()
{
	/* EnemyObj.cs line 191. The stat-per-level gains apply when the castle
	 * carries room levels (TODO with the castle level scaling). */
	InitializeBaseEV();
	InitializeEV();
	if (NumChildren() > 0)
	{
		/* TintablePart is the first child (Initialize); the tint is read
		 * after InitializeEV so the difficulty tint sticks. */
		m_baseTint = GetChildAt(0)->TextureColor();
	}
	m_initialDelayCounter = 0;
	CurrentHealth = MaxHealth;
	m_resetPos = Position();
}

void EnemyObj::Update(GameTime *gameTime)
{
	/* EnemyObj.cs line 254 (Update). */
	float elapsedSeconds = (float) gameTime->ElapsedGameTime;

	if (m_initialDelayCounter > 0)
	{
		m_initialDelayCounter -= elapsedSeconds;
		return;
	}

	if (m_invincibleCounter > 0)
		m_invincibleCounter -= elapsedSeconds;
	if (m_invincibleCounterProjectile > 0)
		m_invincibleCounterProjectile -= elapsedSeconds;

	/* The damage blink. */
	if (m_blinkCounter > 0)
		m_blinkCounter -= elapsedSeconds;
	if (NumChildren() > 0)
		GetChildAt(0)->TextureColor(m_blinkCounter > 0 ? m_blinkColour : m_baseTint);

	/* Flying enemies: ease the acceleration back to zero. */
	if (!IsWeightedFlag && !IsInvincible())
	{
		if (AccelerationY() < 0)
			AccelerationY(AccelerationY() + 15.0f);
		else if (AccelerationY() > 0)
			AccelerationY(AccelerationY() - 15.0f);
		if (AccelerationX() < 0)
			AccelerationX(AccelerationX() + 15.0f);
		else if (AccelerationX() > 0)
			AccelerationX(AccelerationX() - 15.0f);
		if (AccelerationY() < 3.6f && AccelerationY() > -3.6f)
			AccelerationY(0);
		if (AccelerationX() < 3.6f && AccelerationX() > -3.6f)
			AccelerationX(0);
	}

	if (!IsKilled && !IsPaused && m_target != nullptr && m_target->CurrentHealth > 0)
	{
		DistanceToPlayer = DistanceBetweenPts(Position(), m_target->Position());
		if (DistanceToPlayer > EngageRadius)
			State = STATE_WANDER;
		else if (DistanceToPlayer < EngageRadius && DistanceToPlayer >= ProjectileRadius)
			State = STATE_ENGAGE;
		else if (DistanceToPlayer < ProjectileRadius && DistanceToPlayer >= MeleeRadius)
			State = STATE_PROJECTILE_ENGAGE;
		else
			State = STATE_MELEE_ENGAGE;

		/* Count down the cooldown timer. */
		if (m_cooldownTimer > 0)
			m_cooldownTimer -= elapsedSeconds;
		if (m_cooldownTimer <= 0 && m_runCooldown)
			m_runCooldown = false;

		/* Facing (EnemyObj.cs line 312). */
		if (AlwaysFaceTarget)
		{
			if (X() > m_target->X())
				Flip(SpriteEffects::FlipHorizontally);
			else
				Flip(SpriteEffects::None);
		}
		else
		{
			if (Heading().X < 0)
				Flip(SpriteEffects::FlipHorizontally);
			else
				Flip(SpriteEffects::None);
		}

		/* Pick a new action when none is running and the cooldown is over. */
		if (!m_actionActive && !m_runCooldown)
		{
			switch (DifficultyType)
			{
			case BASIC: RunBasicLogic(); break;
			case ADVANCED: RunAdvancedLogic(); break;
			case EXPERT: RunExpertLogic(); break;
			case MINIBOSS: RunMinibossLogic(); break;
			}
		}
		if (m_actionTimer > 0)
			m_actionTimer -= elapsedSeconds;
		if (m_actionActive && ActionComplete())
		{
			m_actionActive = false;
			m_actionPhase = 0;
			CurrentSpeed(0);
		}

		/* Movement (EnemyObj.cs line 355). */
		if (IsWeightedFlag && !IsInvincible())
		{
			if (HeadingX() > 0)
				HeadingX(1);
			else if (HeadingX() < 0)
				HeadingX(-1);
			X(X() + HeadingX() * (CurrentSpeed() * elapsedSeconds));
		}
		else if (m_isTouchingGround || !IsWeightedFlag)
			Position(Position() + Heading() * (CurrentSpeed() * elapsedSeconds));

		/* Code to make sure enemies cannot exit the room. */
		if (m_levelScreen != nullptr && m_levelScreen->CurrentRoom() != nullptr)
		{
			Rectangle b = m_levelScreen->CurrentRoom()->Bounds();
			if (X() < b.Left())
				X((float) b.Left());
			else if (X() > b.Right())
				X((float) b.Right());
			if (Y() < b.Top())
				Y((float) b.Top());
			else if (Y() > b.Bottom())
				Y((float) b.Bottom());
		}

		UpdateEnemy(gameTime);
	}

	if (IsWeightedFlag)
		CheckGroundCollision();

	if (CurrentHealth <= 0 && !IsKilled)
		Kill(true);
}

float EnemyObj::ParseTagToFloat(const std::string &key)
{
	/* EnemyObj.cs line 1140. No exceptions: a missing or malformed value
	 * returns 0 (the C# catches the float.Parse failure). */
	if (Tag.empty())
		return 0;
	size_t keyPos = Tag.find(key + ":");
	if (keyPos == std::string::npos)
		return 0;
	size_t firstIndex = keyPos + key.length() + 1;
	size_t comma = Tag.find(',', firstIndex);
	if (comma == std::string::npos)
		comma = Tag.length();
	return (float) atof(Tag.substr(firstIndex, comma - firstIndex).c_str());
}

void EnemyObj::CheckGroundCollision()
{
	/* Simplified port of CheckGroundCollision (EnemyObj.cs line 458): a
	 * probe under the terrain bounds, plus the ledge turn-around for
	 * enemies that cannot fall off ledges. */
	m_isTouchingGround = false;
	if (m_attachedTerrain == nullptr)
		return;
	Rectangle tb = TerrainBounds();
	Rectangle probe(tb.X + 1, tb.Bottom() - 1, tb.Width > 2 ? tb.Width - 2 : 1, 11);
	for (const IPhysicsObj *terrain : *m_attachedTerrain)
	{
		if (!terrain->CollidesTop())
			continue;
		if (!probe.Intersects(terrain->TerrainBounds()))
			continue;
		m_isTouchingGround = true;
		break;
	}

	if (m_isTouchingGround && !CanFallOffLedges && CurrentSpeed() > 0)
	{
		Rectangle ahead(HeadingX() > 0 ? tb.Right() : tb.X - 12, tb.Bottom() - 1, 12, 11);
		bool groundAhead = false;
		for (const IPhysicsObj *terrain : *m_attachedTerrain)
		{
			if (!terrain->CollidesTop())
				continue;
			if (ahead.Intersects(terrain->TerrainBounds()))
			{
				groundAhead = true;
				break;
			}
		}
		if (!groundAhead)
			HeadingX(-HeadingX());   /* turn around at the ledge */
	}
}

void EnemyObj::Blink(Color colour, float blinkTime)
{
	m_blinkColour = colour;
	m_blinkCounter = blinkTime;
}

void EnemyObj::HitEnemy(int damage, Vector2 collisionPt, bool isPlayer)
{
	/* EnemyObj.cs line 808. */
	(void) collisionPt;
	if (m_target == nullptr || m_target->CurrentHealth <= 0)
		return;

	Blink(Color(255, 0, 0), 0.1f);
	CurrentHealth -= damage;

	if (isPlayer)
	{
		m_target->NumSequentialAttacks++;
		/* Upwards air bounce: only one object bounces the player. */
		if (m_target->IsAirAttacking)
		{
			m_target->IsAirAttacking = false;
			m_target->AccelerationY(-m_target->AirAttackKnockBack);
		}
	}

	const Lineage::Heir *hero = m_target->Stats() != nullptr ? &m_target->Stats()->hero : nullptr;
	if (CanBeKnockedBack && !IsPaused && (hero == nullptr || !hero->HasTrait(TraitType::Hypogonadism)))
	{
		CurrentSpeed(0);
		float knockbackMod = hero != nullptr && hero->HasTrait(TraitType::Hypergonadism) ? GameEV::TRAIT_HYPERGONADISM : 1;
		if (KnockBack == Vector2(0, 0))
		{
			if (X() < m_target->X())
				AccelerationX(-m_target->EnemyKnockBack.X * knockbackMod);
			else
				AccelerationX(m_target->EnemyKnockBack.X * knockbackMod);
			AccelerationY(-m_target->EnemyKnockBack.Y * knockbackMod);
		}
		else
		{
			if (X() < m_target->X())
				AccelerationX(-KnockBack.X * knockbackMod);
			else
				AccelerationX(KnockBack.X * knockbackMod);
			AccelerationY(-KnockBack.Y * knockbackMod);
		}
	}
}

void EnemyObj::Kill(bool giveXP)
{
	/* EnemyObj.cs line 881. TODO C++: XP, gold and item drops
	 * (ItemDropManager), the death animation, KillEnemy on the level. */
	(void) giveXP;
	if (IsKilled)
		return;
	IsKilled = true;
	Visible(false);
	IsCollidableFlag = false;
	StopAnimation();
	/* EnemyObj.cs line 906. */
	Audio::PlaySound("Enemy_Death");

	/* EnemyObj.cs lines 908-960: the item and money drops. */
	if (DropsItem && m_levelScreen != nullptr)
	{
		if (Type == (int) EnemyType::Chicken)
			m_levelScreen->DropItem(Position(), ItemDropType::Health,
			                        GameEV::ITEM_HEALTHDROP_AMOUNT);
		else if (rand() % 100 + 1 <= GameEV::ENEMY_ITEMDROP_CHANCE)
		{
			if (rand() % 2 == 0)
				m_levelScreen->DropItem(Position(), ItemDropType::Health,
				                        GameEV::ITEM_HEALTHDROP_AMOUNT);
			else
				m_levelScreen->DropItem(Position(), ItemDropType::Mana,
				                        GameEV::ITEM_MANADROP_AMOUNT);
		}

		/* EnemyObj.cs line 922: goldAmount, then broken down into the
		 * biggest drops first (big diamonds, diamonds, bags, coins). */
		float roll = (float) rand() / (float) RAND_MAX;
		if (roll <= MoneyDropChance)
		{
			int goldAmount = (rand() % (MaxMoneyDropAmount - MinMoneyDropAmount + 1)
			                  + MinMoneyDropAmount) * 10
			                 + (int) ((((float) rand() / (float) RAND_MAX
			                            * (MaxMoneyGainPerLevel - MinMoneyGainPerLevel)
			                            + MinMoneyGainPerLevel) * Level) * 10);

			int numBigDiamonds = goldAmount / ItemDropType::BigDiamondAmount;
			goldAmount -= numBigDiamonds * ItemDropType::BigDiamondAmount;

			int numDiamonds = goldAmount / ItemDropType::DiamondAmount;
			goldAmount -= numDiamonds * ItemDropType::DiamondAmount;

			int numMoneyBags = goldAmount / ItemDropType::MoneyBagAmount;
			goldAmount -= numMoneyBags * ItemDropType::MoneyBagAmount;

			int numCoins = goldAmount / ItemDropType::CoinAmount;

			for (int i = 0; i < numBigDiamonds; i++)
				m_levelScreen->DropItem(Position(), ItemDropType::BigDiamond,
				                        ItemDropType::BigDiamondAmount);
			for (int i = 0; i < numDiamonds; i++)
				m_levelScreen->DropItem(Position(), ItemDropType::Diamond,
				                        ItemDropType::DiamondAmount);
			for (int i = 0; i < numMoneyBags; i++)
				m_levelScreen->DropItem(Position(), ItemDropType::MoneyBag,
				                        ItemDropType::MoneyBagAmount);
			for (int i = 0; i < numCoins; i++)
				m_levelScreen->DropItem(Position(), ItemDropType::Coin,
				                        ItemDropType::CoinAmount);
		}
	}
}

void EnemyObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType)
{
	/* EnemyObj.cs line 651. */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_FIRSTBOXHIT)
	{
		IPhysicsObj *otherParent = otherBox->PhysicsParent();
		/* The player (sword) and the player's spell projectiles both
		 * carry tag 2; the spell explosions tag 4 (GLOBAL_DAMAGE_WALL).
		 * EnemyObj.cs line 655. */
		if (otherParent != nullptr
		    && (otherParent->CollisionTypeTag() == 2 || otherParent->CollisionTypeTag() == 4))
		{
			/* EnemyObj.cs line 660: a hallucination only shows a "?" */
			if (Demented)
			{
				if (m_invincibleCounter <= 0 && m_invincibleCounterProjectile <= 0 && m_levelScreen != nullptr)
					m_levelScreen->AddPopup("?", X(), (float) Bounds().Top(), 0xFFFFFFFFu);
				m_invincibleCounter = m_invincibleCounterProjectile = InvincibilityTime;
				return;
			}
			bool isProjectile = otherParent->IsSpellProjectile();
			float invincible = isProjectile ? m_invincibleCounterProjectile
			                                : m_invincibleCounter;
			if (invincible <= 0)
			{
				Vector2 impact((float) thisBox->AbsRect().X, (float) thisBox->AbsRect().Y);
				if (isProjectile && otherParent->ProjectileSpell() == SpellType::Shout)
				{
					/* The Barbarian shout: knockback only (EnemyObj.cs
					 * line 707), with the shout's knockback mod. */
					if (CanBeKnockedBack && !IsPaused && m_target != nullptr)
					{
						CurrentSpeed(0);
						float mod = GameEV::BARBARIAN_SHOUT_KNOCKBACK_MOD;
						Vector2 kb = KnockBack != Vector2(0, 0)
						                 ? KnockBack : m_target->EnemyKnockBack;
						if (X() < m_target->X())
							AccelerationX(-kb.X * mod);
						else
							AccelerationX(kb.X * mod);
						AccelerationY(-kb.Y * mod);
					}
				}
				else
				{
					/* EnemyObj.cs line 815. */
					Audio::PlaySound("EnemyHit1", "EnemyHit2", "EnemyHit3");
					int damage = otherParent->DealsDamage();
					if (isProjectile)
						m_invincibleCounterProjectile = InvincibilityTime;
					else
						m_invincibleCounter = InvincibilityTime;
					HitEnemy(damage, impact, true);
					otherParent->OnEnemyHit();
				}
			}
		}
		/* Enemy weapons do nothing to other enemies. */
		return;
	}

	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN)
		PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

void EnemyObj::ResetState()
{
	/* EnemyObj.cs line 1113: the state to reset the enemy to when
	 * transitioning rooms. */
	Position(m_resetPos);
	Acceleration(Vector2(0, 0));
	CurrentSpeed(0);
	Heading(Vector2(0, 0));
	IsKilled = false;
	Visible(true);
	IsCollidableFlag = true;
	CurrentHealth = MaxHealth;
	m_invincibleCounter = 0;
	m_invincibleCounterProjectile = 0;
	m_runCooldown = false;
	m_cooldownTimer = 0;
	m_actionActive = false;
	m_actionTimer = 0;
	ChangeSprite(m_resetSpriteName);
	PlayAnimation(true);
}

} // namespace RogueCastle
