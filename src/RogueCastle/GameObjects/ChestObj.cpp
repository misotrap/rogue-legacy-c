/* Port of RogueCastle/src/Game Objects/ChestObj.cs. */
#include "ChestObj.h"
#include "Player/PlayerObj.h"
#include "ItemDropObj.h"
#include "../EVs/GameEV.h"
#include "../Types/ChestType.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "../../Audio/Audio.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>

namespace RogueCastle
{

using namespace DS2DEngine;
/* The member accessor ChestType() shadows the namespace inside the class. */
namespace CT = ChestType;

/* The C# CDGMath generator drives the chest rolls; the port shares the
 * libc rand() used by the rest of the loot code. */
static int RandomInt(int min, int max)
{
	return rand() % (max - min + 1) + min;
}

ChestObj::ChestObj()
	: PhysicsObj("Chest1_Sprite")
{
	/* ChestObj.cs constructor: not weighted, one frame shown. */
	IsWeighted = false;
	IsCollidable = false;
	m_arrowIcon = new SpriteObj("UpArrowSquare_Sprite");
	m_arrowIcon->Visible(false);
}

void ChestObj::SetChestType(uint8_t type)
{
	/* ChestObj.cs line 392 (ChestType setter): the sprite follows the
	 * colour; an open chest stays open. Boss chests force a triple stat
	 * drop (ItemDropType.TripStatDrop). */
	bool openChest = IsOpen();
	m_chestType = type;
	if (type == CT::Boss)
	{
		ForcedItemType = ItemDropType::TripStatDrop;
		ChangeSprite("BossChest_Sprite");
	}
	else if (type == CT::Fairy)
		ChangeSprite("Chest4_Sprite");
	else if (type == CT::Gold)
		ChangeSprite("Chest3_Sprite");
	else if (type == CT::Silver)
		ChangeSprite("Chest2_Sprite");
	else
		ChangeSprite("Chest1_Sprite");
	if (openChest)
		GoToFrame(2);
}

void ChestObj::Update(GameTime *gameTime)
{
	/* Game.TotalGameTime, kept by the chest itself (the arrow bob). */
	m_totalTime += (float) gameTime->ElapsedGameTime;
}

void ChestObj::OpenChest(ProceduralLevelScreen *level, PlayerObj *player)
{
	/* ChestObj.cs line 44 (OpenChest). */
	if (IsOpen() || IsLocked || level == nullptr)
		return;
	Audio::PlaySound("Chest_Open_Large");
	GoToFrame(2);
	if (IsEmpty)
		return;

	if (ForcedItemType == 0)
	{
		/* Item drop chance for MONEY/STAT DROP/BLUEPRINT. */
		int dropRoll = RandomInt(1, 100);
		const int *dropChance = GameEV::BRONZECHEST_ITEMDROP_CHANCE;
		if (ChestType() == CT::Brown)
			dropChance = GameEV::BRONZECHEST_ITEMDROP_CHANCE;
		else if (ChestType() == CT::Silver)
			dropChance = GameEV::SILVERCHEST_ITEMDROP_CHANCE;
		else
			dropChance = GameEV::GOLDCHEST_ITEMDROP_CHANCE;

		int itemType = 0, itemChance = 0;
		for (int i = 0; i < 3; i++)
		{
			itemChance += dropChance[i];
			if (dropRoll <= itemChance)
			{
				itemType = i;
				break;
			}
		}

		if (itemType == 0)
			GiveGold(level);
		else if (itemType == 1)
			GiveStatDrop(level, player, 1, 0);
		else
			GivePrint(level, player);
	}
	else
	{
		switch (ForcedItemType)
		{
		case ItemDropType::Coin:
		case ItemDropType::MoneyBag:
		case ItemDropType::Diamond:
		case ItemDropType::BigDiamond:
			GiveGold(level, (int) ForcedAmount);
			break;
		case ItemDropType::Stat_Defense:
		case ItemDropType::Stat_Magic:
		case ItemDropType::Stat_MaxHealth:
		case ItemDropType::Stat_MaxMana:
		case ItemDropType::Stat_Weight:
		case ItemDropType::Stat_Strength:
			GiveStatDrop(level, player, 1, ForcedItemType);
			break;
		case ItemDropType::Blueprint:
		case ItemDropType::Redprint:
			GivePrint(level, player);
			break;
		case ItemDropType::TripStatDrop:
			/* Boss chests: 3 random stat boosts. */
			GiveStatDrop(level, player, 3, 0);
			break;
		default:
			/* The fountain pieces (not in the castle rooms). */
			GiveStatDrop(level, player, 1, ForcedItemType);
			break;
		}
	}
}

void ChestObj::GiveGold(ProceduralLevelScreen *level, int amount)
{
	/* ChestObj.cs line 154 (GiveGold): gold per chest colour plus the
	 * level increase (GoldIncreasePerLevel 1.425), then broken into the
	 * biggest drops first. The C# delays each drop by 0.1s; the port
	 * drops them together. */
	int goldAmount = 0;
	if (ChestType() == CT::Brown)
		goldAmount = RandomInt(9, 14) * 10;   /* BronzeChestGoldRange */
	else if (ChestType() == CT::Silver || ChestType() == CT::Fairy)
		goldAmount = RandomInt(20, 28) * 10;  /* SilverChestGoldRange */
	else
		goldAmount = RandomInt(47, 57) * 10;  /* GoldChestGoldRange */
	goldAmount += (int) (1.425f * Level * 10);

	if (amount != 0)
		goldAmount = amount;

	int numBigDiamonds = goldAmount / ItemDropType::BigDiamondAmount;
	goldAmount -= numBigDiamonds * ItemDropType::BigDiamondAmount;
	int numDiamonds = goldAmount / ItemDropType::DiamondAmount;
	goldAmount -= numDiamonds * ItemDropType::DiamondAmount;
	int numMoneyBags = goldAmount / ItemDropType::MoneyBagAmount;
	goldAmount -= numMoneyBags * ItemDropType::MoneyBagAmount;
	int numCoins = goldAmount / ItemDropType::CoinAmount;

	Vector2 pos(X(), Y() - Bounds().Height * 0.5f);
	for (int i = 0; i < numBigDiamonds; i++)
		level->DropItem(pos, ItemDropType::BigDiamond, ItemDropType::BigDiamondAmount);
	for (int i = 0; i < numDiamonds; i++)
		level->DropItem(pos, ItemDropType::Diamond, ItemDropType::DiamondAmount);
	for (int i = 0; i < numMoneyBags; i++)
		level->DropItem(pos, ItemDropType::MoneyBag, ItemDropType::MoneyBagAmount);
	for (int i = 0; i < numCoins; i++)
		level->DropItem(pos, ItemDropType::Coin, ItemDropType::CoinAmount);
}

void ChestObj::GiveStatDrop(ProceduralLevelScreen *level, PlayerObj *player,
                            int numDrops, int statDropType)
{
	/* ChestObj.cs line 214 (GiveStatDrop): the bonus is granted at once
	 * (the C# shows the reward through the GetItem screen, which the
	 * port replaces with the popup text, see the header). */
	if (player == nullptr || player->Stats() == nullptr)
		return;
	Save::GameState *stats = player->Stats();
	for (int k = 0; k < numDrops; k++)
	{
		int dropType = statDropType;
		if (statDropType == 0)
		{
			/* STR/MAG/DEF/HP/MP/WEIGHT, GameEV.STATDROP_CHANCE. */
			int dropRoll = RandomInt(1, 100), chance = 0;
			const int statTypes[6] = { ItemDropType::Stat_Strength, ItemDropType::Stat_Magic,
			                           ItemDropType::Stat_Defense, ItemDropType::Stat_MaxHealth,
			                           ItemDropType::Stat_MaxMana, ItemDropType::Stat_Weight };
			for (int i = 0; i < 6; i++)
			{
				chance += GameEV::STATDROP_CHANCE[i];
				if (dropRoll <= chance)
				{
					dropType = statTypes[i];
					break;
				}
			}
		}

		const char *text = "";
		switch (dropType)
		{
		case ItemDropType::Stat_MaxHealth:
			stats->BonusHealth += 1 + stats->TimesCastleBeaten;
			player->CurrentHealth += GameEV::ITEM_STAT_MAXHP_AMOUNT;
			text = "LOC_ID_ITEM_DROP_OBJ_5";
			break;
		case ItemDropType::Stat_MaxMana:
			stats->BonusMana += 1 + stats->TimesCastleBeaten;
			text = "LOC_ID_ITEM_DROP_OBJ_6";
			break;
		case ItemDropType::Stat_Defense:
			stats->BonusDefense += 1 + stats->TimesCastleBeaten;
			text = "LOC_ID_ITEM_DROP_OBJ_4";
			break;
		case ItemDropType::Stat_Magic:
			stats->BonusMagic += 1 + stats->TimesCastleBeaten;
			text = "LOC_ID_ITEM_DROP_OBJ_8";
			break;
		case ItemDropType::Stat_Weight:
			stats->BonusWeight += 1 + stats->TimesCastleBeaten;
			text = "LOC_ID_ITEM_DROP_OBJ_9";
			break;
		default:
			stats->BonusStrength += 1 + stats->TimesCastleBeaten;
			text = "LOC_ID_ITEM_DROP_OBJ_7";
			break;
		}
		level->AddPopup(text, X(), Y() - Bounds().Height,
		                0xFFB0C4DEu /* Color.LightSteelBlue */);
	}
}

void ChestObj::GivePrint(ProceduralLevelScreen *level, PlayerObj *player)
{
	/* ChestObj.cs line 260 (GivePrint): a random blueprint this chest's
	 * level and colour can give; gold if none is left. */
	if (player == nullptr || player->Stats() == nullptr)
		return;
	Save::GameState *stats = player->Stats();
	std::mt19937 rng((unsigned) rand());
	int category = 0, index = 0;
	if (stats->equipment.GiveRandomBlueprint(Level, (int) ChestType(), rng, category, index))
	{
		/* The blacksmith screen shows it as a new (unseen) blueprint. */
		level->AddPopup("LOC_ID_ITEM_DROP_OBJ_4", X(), Y() - Bounds().Height,
		                0xFFB0C4DEu);
		if (getenv("RL_DEBUG_TRACE") != nullptr)
			fprintf(stderr, "chest: blueprint %d of category %d unlocked\n",
			        index, category);
	}
	else
		GiveGold(level);
}

void ChestObj::Draw(Camera2D *camera)
{
	/* ChestObj.cs line 326 (Draw): the bobbing arrow over the chest. */
	if (ArrowVisible)
	{
		Rectangle b = Bounds();
		m_arrowIcon->X(b.X + b.Width * 0.5f);
		m_arrowIcon->Y((float) b.Top() - 50 + sinf(m_totalTime * 20) * 3);
		m_arrowIcon->Visible(true);
		m_arrowIcon->Draw(camera);
		m_arrowIcon->Visible(false);
		ArrowVisible = false;
	}
	PhysicsObj::Draw(camera);
}

} // namespace RogueCastle
