/* The folding platform (EnemyObj_Platform.cs): a down strike on its button
 * unfolds the plate, a floor open underneath for 3 s; it blinks red for
 * the last 1.5 s, then folds back. The PlatformsOpen trait keeps them all
 * unfolded. */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_PLATFORM_H
#define ROGUE_CASTLE_ENEMY_OBJ_PLATFORM_H

#include "EnemyObj.h"
#include "../TerrainObj.h"

namespace RogueCastle
{

/* The floor under the unfolded plate: an ordinary one-way platform
 * (open underneath), switched on and off with the plate */
class PlatformFloor : public TerrainObj
{
public:
	PlatformFloor() : TerrainObj(1, 1) {}
	bool Enabled = false;
	bool IsStatic() const override { return false; }
	bool IsCollidable() const override { return Enabled; }
	bool CollidesTop() const override { return Enabled; }
	bool CollidesBottom() const override { return false; }
	bool CollidesLeft() const override { return false; }
	bool CollidesRight() const override { return false; }
};

class EnemyObj_Platform : public EnemyObj
{
public:
	EnemyObj_Platform(PlayerObj *target, EnemyDifficulty difficulty);

	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	void ResetState() override;
	~EnemyObj_Platform() override { delete m_floor; }
	/* Its floor, for the level's physics and ground lists */
	PlatformFloor *Floor() { return m_floor; }

protected:
	void InitializeEV() override;
	void RunBasicLogic() override {}
	void UpdateEnemy(GameTime *gameTime) override;

private:
	PlatformFloor *m_floor = new PlatformFloor();
	float m_extendTime = 0;   // since the hit: the floor is there once the plate is out
	bool m_isExtended = false;
	void UpdateFloor(float dt);
	bool m_blinkedWarning = false;
	float m_retractCounter = 0;
	float m_warningTimer = 0;
	float m_foldTimer = 0;      // folding back: no new hit until it is done   // the 10 red blinks, 0.1 s apart
	int m_warningBlinks = 0;
	bool PlatformsOpen() const;
};

} // namespace RogueCastle

#endif
