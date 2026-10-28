/* Port of RogueCastle/src/Game Objects/FairyChestObj.cs: the fairy chest,
 * unlocked by doing (or avoiding) what its condition says
 * (ChestConditionChecker). Opened like a chest, but it always gives a rune
 * (or a stat drop / gold when every rune has been found). */
#ifndef ROGUE_CASTLE_FAIRY_CHEST_OBJ_H
#define ROGUE_CASTLE_FAIRY_CHEST_OBJ_H

#include "ChestObj.h"

namespace RogueCastle
{

class FairyChestObj : public ChestObj
{
public:
	FairyChestObj();

	/* ChestConditionChecker.cs state constants. */
	static const int STATE_LOCKED = 0;
	static const int STATE_FREE = 1;
	static const int STATE_FAILED = 2;

	int State = STATE_LOCKED;
	float Timer = 0;

	/* FairyChestObj.cs line 38: the condition comes from the Tag
	 * ("1" = kill all enemies...), unless the level forces one. */
	void SetConditionType(int conditionType = 0);
	int ConditionType() const { return m_conditionType; }

	void SetChestUnlocked();
	void SetChestFailed();

	void OpenChest(class ProceduralLevelScreen *level, class PlayerObj *player) override;
	void ForceOpen() override;
	bool IsFairyChest() const override { return true; }
	void Update(GameTime *gameTime) override;
	void Draw(DS2DEngine::Camera2D *camera) override;

	/* The Tag of the room object (the condition number). */
	std::string Tag;

private:
	int m_conditionType = 0;
	DS2DEngine::SpriteObj *m_lockSprite = nullptr;
	DS2DEngine::SpriteObj *m_errorSprite = nullptr;
};

}

#endif
