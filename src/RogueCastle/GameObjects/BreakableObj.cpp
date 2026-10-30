/* Port of RogueCastle/src/Game Objects/BreakableObj.cs. */
#include "BreakableObj.h"
#include "../../DS2DEngine/Consts.h"
#include "Player/PlayerObj.h"
#include "ItemDropObj.h"
#include "../EVs/GameEV.h"
#include "../Types/TraitType.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "../../Audio/Audio.h"
#include <cstdlib>

namespace RogueCastle
{

using namespace DS2DEngine;

BreakableObj::BreakableObj(const std::string &spriteName)
	: PhysicsObjContainer(spriteName)
{
	/* BreakableObj.cs constructor. */
	CollisionTypeTag(5);       /* GameTypes.CollisionType_WALL_FOR_PLAYER */
	SameTypesCollideFlag = true;
	CollidesLeftFlag = false;
	CollidesRightFlag = false;
	CollidesBottomFlag = false;

	/* Only the first part shows while intact; Break() shows the rest. */
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->Visible(i == 0);
}

void BreakableObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType)
{
	/* BreakableObj.cs line 36. */
	if (otherBox->PhysicsParent() != nullptr && otherBox->Type() == Consts::WEAPON_HITBOX
	    && !HitBySpellsOnly)
	{
		if (!Broken && otherBox->PhysicsParent()->CollisionTypeTag() == 2 /* player */)
		{
			Break();
			/* PlayerObj.cs line 1949: a down-strike bounces off it */
			PlayerObj *player = m_levelScreen != nullptr ? m_levelScreen->Player() : nullptr;
			if (player != nullptr && otherBox->PhysicsParent() == (DS2DEngine::IPhysicsObj *) player && player->IsAirAttacking)
			{
				player->IsAirAttacking = false;
				player->NumAirBounces++;
				player->AccelerationY(-player->AirAttackKnockBack);
			}
		}
	}

	/* BreakableObj.cs line 52: the player's spell projectiles break even
	 * the HitBySpellsOnly targets (the carnival's bullseyes). */
	if (otherBox->PhysicsParent() != nullptr && otherBox->PhysicsParent()->IsSpellProjectile()
	    && otherBox->PhysicsParent()->CollisionTypeTag() == 2 /* player */
	    && otherBox->Type() == Consts::WEAPON_HITBOX && !Broken)
	{
		Break();
	}

	/* Hack to make only terrain or other breakables collide with this:
	 * pushed only by something above (or rotated, not supported). */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN
	    && otherBox->AbsRect().Y >= thisBox->AbsRect().Y)
		PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

void BreakableObj::Break()
{
	/* BreakableObj.cs lines 61-147. */
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->Visible(true);

	GoToFrame(2);
	Broken = true;
	m_pieces.clear();
	for (int i = 0; i < NumChildren(); i++)
		m_pieces.push_back({ GetChildAt(i)->X(), GetChildAt(i)->Y(), (float) (rand() % 101 - 50), (float) (rand() % 721 - 360) });
	m_breakTime = 0;
	m_wasWeighted = IsWeightedFlag;
	IsWeightedFlag = false;
	IsCollidableFlag = false;

	if (!DropItem || m_levelScreen == nullptr || m_levelScreen->Player() == nullptr)
		return;
	PlayerObj *player = m_levelScreen->Player();

	/* BreakableObj.cs line 80: "Health"/"Mana" breakables force a drop. */
	bool droppedItem = false;
	if (Name == "Health")
	{
		m_levelScreen->DropItem(Position(), ItemDropType::Health,
		                         GameEV::ITEM_HEALTHDROP_AMOUNT);
		droppedItem = true;
	}
	else if (Name == "Mana")
	{
		m_levelScreen->DropItem(Position(), ItemDropType::Mana,
		                         GameEV::ITEM_MANADROP_AMOUNT);
		droppedItem = true;
	}

	if (droppedItem)
	{
		Audio::PlaySound("EnemyHit1", "EnemyHit2", "EnemyHit3");
		Audio::PlaySound("Break1", "Break2", "Break3");
		return;
	}

	/* BreakableObj.cs line 111: the drop roll over
	 * BREAKABLE_ITEMDROP_CHANCE {3,4,36,1,56} (health, mana, coin, bag,
	 * nothing). Alektorophobia swaps the health potion for a chicken
	 * (TODO C++: enemy spawning from a breakable). */
	int dropRoll = rand() % 100 + 1;
	int dropChance = 0;
	for (int i = 0; i < 5; i++)
	{
		dropChance += GameEV::BREAKABLE_ITEMDROP_CHANCE[i];
		if (dropRoll <= dropChance)
		{
			if (i == 0)
			{
				if (!player->Stats()->hero.HasTrait(TraitType::Alektorophobia))
					m_levelScreen->DropItem(Position(), ItemDropType::Health,
					                         GameEV::ITEM_HEALTHDROP_AMOUNT);
			}
			else if (i == 1)
				m_levelScreen->DropItem(Position(), ItemDropType::Mana,
				                         GameEV::ITEM_MANADROP_AMOUNT);
			else if (i == 2)
				m_levelScreen->DropItem(Position(), ItemDropType::Coin,
				                         ItemDropType::CoinAmount);
			else if (i == 3)
				m_levelScreen->DropItem(Position(), ItemDropType::MoneyBag,
				                         ItemDropType::MoneyBagAmount);
			break;
		}
	}

	Audio::PlaySound("EnemyHit1", "EnemyHit2", "EnemyHit3");
	Audio::PlaySound("Break1", "Break2", "Break3");

	/* BreakableObj.cs line 145: the OCD trait gives 1 mp per breakable. */
	if (player->Stats()->hero.HasTrait(TraitType::OCD))
	{
		player->CurrentMana += 1;
		m_levelScreen->AddPopup("1 mp", player->X(),
		                        (float) player->Bounds().Top() - 30,
		                        0xFF4169E1u /* Color.RoyalBlue */);
	}
}

} // namespace RogueCastle

namespace RogueCastle
{
void BreakableObj::Draw(DS2DEngine::Camera2D *camera)
{
	/* Tween.By X +-50, Y +50, Rotation +-360 over 0.3 s; Opacity to 0
	 * from 0.2 to 0.3 s. The pieces are in the parent's scaled space. */
	if (m_breakTime >= 0 && m_breakTime < 0.3f)
	{
		m_breakTime += (float) camera->ElapsedTotalSeconds();
		float t = m_breakTime < 0.3f ? m_breakTime / 0.3f : 1;
		for (int i = 0; i < NumChildren() && i < (int) m_pieces.size(); i++)
		{
			const Piece &p = m_pieces[i];
			GameObj *child = GetChildAt(i);
			child->X(p.x + p.dx * t / ScaleX());
			child->Y(p.y + 50 * t / ScaleY());
			child->Rotation(p.rotation * t);
			child->Opacity(m_breakTime < 0.2f ? 1 : 1 - (m_breakTime - 0.2f) / 0.1f);
		}
		if (m_breakTime >= 0.3f)
			for (int i = 0; i < NumChildren(); i++) GetChildAt(i)->Visible(false);
	}
	PhysicsObjContainer::Draw(camera);
}
}
