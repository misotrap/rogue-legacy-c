/* The folding platform (EnemyObj_Platform.cs): a down strike on its button
 * unfolds the plate, a floor open underneath for 3 s; it blinks red for
 * the last 1.5 s, then folds back. The PlatformsOpen trait keeps them all
 * unfolded. */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_PLATFORM_H
#define ROGUE_CASTLE_ENEMY_OBJ_PLATFORM_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Platform : public EnemyObj
{
public:
	EnemyObj_Platform(PlayerObj *target, EnemyDifficulty difficulty);

	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	void ResetState() override;
	/* A floor only once unfolded: the plate's top, open underneath */
	bool CollidesTop() const override { return m_isExtended && CurrentFrame() >= EXTENDED_FRAME && CurrentFrame() < EXTENDED_FRAME + 2; }
	bool CollidesBottom() const override { return false; }
	bool CollidesLeft() const override { return false; }
	bool CollidesRight() const override { return false; }
	Rectangle TerrainBounds() const override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override {}
	void UpdateEnemy(GameTime *gameTime) override;

private:
	static const int EXTENDED_FRAME = 5;   // EndRetract: the plate fully out
	bool m_isExtended = false;
	bool m_blinkedWarning = false;
	float m_retractCounter = 0;
	float m_warningTimer = 0;   // the 10 red blinks, 0.1 s apart
	int m_warningBlinks = 0;
	bool PlatformsOpen() const;
};

} // namespace RogueCastle

#endif
