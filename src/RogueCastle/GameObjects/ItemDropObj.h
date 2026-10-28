/* Port of RogueCastle/src/Game Objects/ItemDropObj.cs: the loot the
 * enemies and the breakables drop (coins, money bags, diamonds, the
 * health/mana potions and the stat boosts). A weighted physics object
 * with a single terrain box; the level screen collects it when the
 * player touches it (the C# collects it through the player's
 * CollisionResponse, line 2110 - the port checks the bounds in the
 * level update, same rule: collectable after CollectionCounter).
 */
#ifndef ROGUE_CASTLE_ITEM_DROP_OBJ_H
#define ROGUE_CASTLE_ITEM_DROP_OBJ_H

#include "../../DS2DEngine/GameObjs/PhysicsObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"

namespace RogueCastle
{

/* ItemDropType.cs (the drop kinds and their gold values). */
namespace ItemDropType
{
	static const int None = 0;
	static const int Health = 2;
	static const int Mana = 3;
	static const int Stat_Strength = 4;
	static const int Stat_Magic = 5;
	static const int Stat_Defense = 6;
	static const int Stat_MaxHealth = 7;
	static const int Stat_MaxMana = 8;
	static const int Stat_Weight = 9;
	static const int Coin = 10;
	static const int MoneyBag = 11;
	static const int Diamond = 12;
	static const int BigDiamond = 13;

	static const int CoinAmount = 10;
	static const int MoneyBagAmount = 100;
	static const int DiamondAmount = 1000;
	static const int BigDiamondAmount = 10000;
}

class ItemDropObj : public DS2DEngine::PhysicsObjContainer
{
public:
	ItemDropObj();

	/* ItemDropObj.cs line 21 (ConvertDrop): the sprite and the amount. */
	void ConvertDrop(int dropType, float amount);
	/* ItemDropObj.cs line 92 (GiveReward): what the player gets. */
	void GiveReward(class PlayerObj *player);

	int DropType = ItemDropType::None;
	float CollectionCounter = 0;
	bool IsCollectable() const { return CollectionCounter <= 0; }
	bool IsCollected = false;
	/* DisplayNumberStringText: the text floats above the drop. */
	std::string PopupText;
	Color PopupColour = Color(255, 255, 255);

	/* The terrain box is the drop's whole sprite (the pickup check runs
	 * on the bounds). */
	Rectangle TerrainBounds() const override
	{
		return Bounds();
	}

private:
	float m_amount = 0;
};

} // namespace RogueCastle

#endif
