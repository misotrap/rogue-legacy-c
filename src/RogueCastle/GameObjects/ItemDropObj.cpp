/* Port of RogueCastle/src/Game Objects/ItemDropObj.cs. */
#include "ItemDropObj.h"
#include "Player/PlayerObj.h"
#include "../Save/SaveGame.h"
#include "../EVs/GameEV.h"
#include "../Skills/SkillData.h"
#include "../../Audio/Audio.h"
#include <cstdio>
#include <cmath>

namespace RogueCastle
{

using namespace DS2DEngine;

ItemDropObj::ItemDropObj()
	: PhysicsObjContainer("")
{
	/* ItemDropObj.cs line 16 (constructor): weighted, enemy-tagged, no
	 * animation yet. */
	IsCollidableFlag = true;
	IsWeightedFlag = true;
	CollisionTypeTag(3);   /* GameTypes.CollisionType_ENEMY */
	StopAnimation();
}

void ItemDropObj::ConvertDrop(int dropType, float amount)
{
	/* ItemDropObj.cs line 21: the sprite per drop type. */
	DropType = dropType;
	m_amount = amount;
	Scale(Vector2(1, 1));
	TextureColor(Color(255, 255, 255));
	const char *sprite = "Coin_Sprite";
	switch (dropType)
	{
	case ItemDropType::Health: sprite = "ChickenLeg_Sprite"; break;
	case ItemDropType::Mana: sprite = "ManaPotion_Sprite"; break;
	case ItemDropType::BigDiamond:
		Scale(Vector2(1.5f, 1.5f));
		TextureColor(Color(152, 251, 152));   /* LightGreen */
		sprite = "Diamond_Sprite";
		break;
	case ItemDropType::Diamond: sprite = "Diamond_Sprite"; break;
	case ItemDropType::MoneyBag: sprite = "MoneyBag_Sprite"; break;
	case ItemDropType::Stat_Strength: sprite = "Sword_Sprite"; break;
	case ItemDropType::Stat_Defense: sprite = "Shield_Sprite"; break;
	case ItemDropType::Stat_Weight: sprite = "Backpack_Sprite"; break;
	case ItemDropType::Stat_MaxMana:
		sprite = "Heart_Sprite";
		TextureColor(Color(0, 0, 255));   /* Blue */
		break;
	case ItemDropType::Stat_MaxHealth: sprite = "Heart_Sprite"; break;
	case ItemDropType::Coin:
	default: sprite = "Coin_Sprite"; break;
	}
	/* One SpriteObj child carries the sprite (the container is the
	 * physics object, like ProjectileObj in this port). */
	if (NumChildren() > 0)
		static_cast<SpriteObj *>(GetChildAt(0))->ChangeSprite(sprite);
	else
		AddChild(new SpriteObj(sprite));
	PlayAnimation(true);
	AnimationDelay(1 / 20.0f);
	/* The terrain box is added by the level screen once the size is
	 * known (after the first UpdateCollisionBoxes). */
}

void ItemDropObj::GiveReward(PlayerObj *player)
{
	/* ItemDropObj.cs line 92 (GiveReward). */
	Save::GameState *stats = player->Stats();
	if (stats == nullptr)
		return;
	switch (DropType)
	{
	case ItemDropType::Coin:
	case ItemDropType::Diamond:
	case ItemDropType::BigDiamond:
	case ItemDropType::MoneyBag:
	{
		/* The gold, times the gold bonus, times the architect's fee
		 * while the locked castle runs (GameEV.ARCHITECT_FEE). */
		float castleLockGoldModifier = 1;
		if (stats->HasArchitectFee)
			castleLockGoldModifier = GameEV::ARCHITECT_FEE;
		int goldAmount = (int) (m_amount * castleLockGoldModifier + 0.5f);
		stats->Gold += goldAmount;
		PopupColour = Color(255, 255, 0);   /* the yellow gold text */
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", goldAmount);
		PopupText = buf;
		break;
	}
	case ItemDropType::Health:
	{
		int healthAmount = (int) (player->MaxHealth()
		                         * (m_amount + stats->skills.ModifierAmount(Skills::SkillType::Potion_Up)));
		player->CurrentHealth += healthAmount;
		PopupColour = Color(124, 252, 0);
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", healthAmount);
		PopupText = buf;
		Audio::PlaySound("Collect_Health");
		break;
	}
	case ItemDropType::Mana:
	{
		int manaAmount = (int) (player->MaxMana()
		                        * (m_amount + stats->skills.ModifierAmount(Skills::SkillType::Potion_Up)));
		player->CurrentMana += manaAmount;
		PopupColour = Color(124, 252, 0);
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", manaAmount);
		PopupText = buf;
		Audio::PlaySound("Collect_Mana");
		break;
	}
	case ItemDropType::Stat_Defense:
		stats->BonusDefense += 1 + stats->TimesCastleBeaten;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_4";   /* Armor Up */
		break;
	case ItemDropType::Stat_MaxHealth:
		stats->BonusHealth += 1 + stats->TimesCastleBeaten;
		player->CurrentHealth += GameEV::ITEM_STAT_MAXHP_AMOUNT;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_5";   /* Max Health Up */
		break;
	case ItemDropType::Stat_MaxMana:
		stats->BonusMana += 1 + stats->TimesCastleBeaten;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_6";   /* Max Mana Up */
		break;
	case ItemDropType::Stat_Strength:
		stats->BonusStrength += 1 + stats->TimesCastleBeaten;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_7";   /* Attack Up */
		break;
	case ItemDropType::Stat_Magic:
		stats->BonusMagic += 1 + stats->TimesCastleBeaten;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_8";   /* Magic Up */
		break;
	case ItemDropType::Stat_Weight:
		stats->BonusWeight += 1 + stats->TimesCastleBeaten;
		PopupColour = Color(176, 196, 222);
		PopupText = "LOC_ID_ITEM_DROP_OBJ_9";   /* Max Weight Up */
		break;
	default:
		break;
	}
}

} // namespace RogueCastle
