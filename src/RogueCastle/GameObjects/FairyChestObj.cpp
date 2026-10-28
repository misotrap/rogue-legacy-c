/* Port of RogueCastle/src/Game Objects/FairyChestObj.cs. */
#include "FairyChestObj.h"
#include "../Screens/GetItemScreen.h"
#include "Player/PlayerObj.h"
#include "ItemDropObj.h"
#include "../ChestConditionChecker.h"
#include "../EVs/GameEV.h"
#include "../Types/ChestType.h"
#include "../Types/ChestConditionType.h"
#include "../Screens/ProceduralLevelScreen.h"
#include "../../Audio/Audio.h"
#include <cstdio>
#include <cstdlib>
#include <random>

namespace RogueCastle
{

using namespace DS2DEngine;
namespace CT = ChestType;

FairyChestObj::FairyChestObj()
	: ChestObj()
{
	/* The lock above the chest and the cross when the player fails. */
	m_lockSprite = new SpriteObj("Chest4Unlock_Sprite");
	m_errorSprite = new SpriteObj("CancelIcon_Sprite");
	m_errorSprite->Visible(false);
	SetChestType(CT::Fairy);
}

void FairyChestObj::SetConditionType(int conditionType)
{
	/* FairyChestObj.cs line 38: the Tag carries the condition number. */
	if (conditionType != 0)
		m_conditionType = conditionType;
	else
		m_conditionType = atoi(Tag.c_str());

	if (m_conditionType == ChestConditionType::ReachIn5Seconds)
		Timer = 5;
}

void FairyChestObj::SetChestUnlocked()
{
	/* FairyChestObj.cs line 56. The objective plate announcement is not
	 * ported yet (TODO C++: the LOC_ID_LEVEL_SCREEN objective text). */
	State = STATE_FREE;
	m_lockSprite->Visible(false);
	m_lockSprite->Opacity(0);
}

void FairyChestObj::SetChestFailed()
{
	/* FairyChestObj.cs line 66. */
	State = STATE_FAILED;
	Audio::PlaySound("FairyChest_Fail");
	m_errorSprite->Visible(true);
	m_errorSprite->Opacity(1);
	m_errorSprite->X(X());
	m_errorSprite->Y(Y() - Bounds().Height * 0.5f);
}

void FairyChestObj::ForceOpen()
{
	/* FairyChestObj.cs line 200: an already-open fairy chest from a save
	 * is free, with no lock or error sprite. */
	State = STATE_FREE;
	m_errorSprite->Visible(false);
	m_lockSprite->Visible(false);
	ChestObj::ForceOpen();
}

void FairyChestObj::Update(GameTime *gameTime)
{
	ChestObj::Update(gameTime);
	/* The C# checks the conditions from Draw; the port checks them from
	 * the level update (same frame rule, see FairyChestObj.cs line 140). */
	if (State == STATE_LOCKED && m_levelScreen != nullptr)
		ChestConditionChecker::SetConditionState(this, m_levelScreen->Player(),
		                                         m_levelScreen);
	if (State == STATE_LOCKED && !IsOpen()
	    && m_conditionType == ChestConditionType::ReachIn5Seconds)
		Timer -= (float) gameTime->ElapsedGameTime;
}

void FairyChestObj::OpenChest(ProceduralLevelScreen *level, PlayerObj *player)
{
	/* FairyChestObj.cs line 76: only when the condition is met, and the
	 * reward is a rune (EquipmentSystem.GiveRandomRune) or a stat drop /
	 * gold once they are all found. */
	if (State != STATE_FREE || IsOpen() || IsLocked)
		return;
	GoToFrame(2);
	Audio::PlaySound("Chest_Open_Large");

	if (player == nullptr || player->Stats() == nullptr)
		return;
	Save::GameState *stats = player->Stats();
	if (stats->equipment.TotalRunesFound() >= Equipment::CATEGORIES * Equipment::ABILITIES)
	{
		GiveStatDrop(level, player, 1, 0);
		return;
	}
	std::mt19937 rng((unsigned) rand());
	int category = 0, ability = 0;
	if (stats->equipment.GiveRandomRune(rng, category, ability))
	{
		/* The enchantress screen shows it as a new (unseen) rune. */
		GetItem::Reward reward;
		reward.type = GetItem::Rune;
		reward.x = category;
		reward.y = ability;
		reward.worldX = X();
		reward.worldY = Y() - Bounds().Height;
		GetItem::Push(reward);
		if (getenv("RL_DEBUG_TRACE") != nullptr)
			fprintf(stderr, "fairy chest: rune %d of category %d unlocked\n",
			        ability, category);
	}
	else
		GiveGold(level);
}

void FairyChestObj::Draw(Camera2D *camera)
{
	/* FairyChestObj.cs line 138 (Draw). The sparkle effect and the
	 * countdown text (ReachIn5Seconds) are TODOs; the timer still runs. */
	if (m_conditionType != ChestConditionType::InvisibleChest || IsOpen())
	{
		ChestObj::Draw(camera);
		m_lockSprite->Flip(Flip());
		if (Flip() == SpriteEffects::None)
			m_lockSprite->Position(Vector2(X() - 10, Y() - Bounds().Height * 0.5f));
		else
			m_lockSprite->Position(Vector2(X() + 10, Y() - Bounds().Height * 0.5f));
		m_lockSprite->Draw(camera);

		m_errorSprite->Position(Vector2(X(), Y() - Bounds().Height * 0.5f));
		m_errorSprite->Draw(camera);
	}
}

} // namespace RogueCastle
