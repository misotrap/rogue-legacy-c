/* Port of RogueCastle/src/Game Objects/ChestObj.cs: the treasure chests.
 * A PhysicsObj whose sprite has 2 frames (closed/open). ProceduralLevelScreen
 * rolls its colour (InitializeChests) and the player opens it with Up
 * (PlayerObj.cs line 2131); the loot rules are OpenChest/GiveGold/
 * GiveStatDrop/GivePrint below.
 *
 * The C# shows the blueprint/rune/stat reward through ScreenType.GetItem;
 * that screen's spritesheet is still DXT5-compressed (AGENTS.md), so the
 * port grants the reward directly and shows the popup text instead.
 */
#ifndef ROGUE_CASTLE_CHEST_OBJ_H
#define ROGUE_CASTLE_CHEST_OBJ_H

#include "../../DS2DEngine/GameObjs/PhysicsObj.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"

namespace RogueCastle
{

class ChestObj : public DS2DEngine::PhysicsObj
{
public:
	ChestObj();

	/* ChestObj.cs line 44 (OpenChest): the loot rolls. */
	virtual void OpenChest(class ProceduralLevelScreen *level, class PlayerObj *player);
	/* Opens the chest without any reward (used by already-open saves). */
	virtual void ForceOpen() { GoToFrame(2); }
	virtual void ResetChest() { GoToFrame(1); }

	void GiveGold(class ProceduralLevelScreen *level, int amount = 0);
	void GiveStatDrop(class ProceduralLevelScreen *level, class PlayerObj *player,
	                  int numDrops, int statDropType);
	void GivePrint(class ProceduralLevelScreen *level, class PlayerObj *player);

	bool IsEmpty = false;
	bool IsLocked = false;
	int ForcedItemType = 0;
	float ForcedAmount = 0;
	bool IsProcedural = true;
	int Level = 0;
	/* The chest's index in the level (save.chestsOpened), set by
	 * InitializeChests. */
	int ChestIndex = -1;
	/* Set once by InitializeChests; false for chests restored from a save. */
	bool IsOpen() const { return CurrentFrame() == 2; }
	virtual bool IsFairyChest() const { return false; }
	/* FairyChestObj: ChestConditionType (0 = a plain chest) */
	virtual int ConditionType() const { return 0; }

	uint8_t ChestType() const { return m_chestType; }
	void SetChestType(uint8_t type);

	/* The "press Up" arrow while the player stands on the chest. */
	bool ArrowVisible = false;
	DS2DEngine::SpriteObj *ArrowIcon() const { return m_arrowIcon; }
	void SetLevelScreen(class ProceduralLevelScreen *levelScreen)
	{
		m_levelScreen = levelScreen;
	}

	void Draw(DS2DEngine::Camera2D *camera) override;
	void Update(GameTime *gameTime) override;

protected:
	uint8_t m_chestType = 0;
	DS2DEngine::SpriteObj *m_arrowIcon = nullptr;
	float m_totalTime = 0;
	class ProceduralLevelScreen *m_levelScreen = nullptr;
};

}

#endif
