/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_BouncySpike.cs:
 * the spiky ball flies in a straight line, bounces off the walls and the
 * room boundaries (heading reflection) and destroys itself after 12
 * bounces within one second. The visual spin (RotationSpeed 250) needs
 * blitter rotation support; the port skips it.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_BOUNCY_SPIKE_H
#define ROGUE_CASTLE_ENEMY_OBJ_BOUNCY_SPIKE_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_BouncySpike : public EnemyObj
{
public:
	EnemyObj_BouncySpike(PlayerObj *target, EnemyDifficulty difficulty);
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                        int collisionResponseType) override;
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override {}
	void RunAdvancedLogic() override {}
	void RunExpertLogic() override {}
	void RunMinibossLogic() override {}
	void UpdateEnemy(GameTime *gameTime) override;

private:
	void ReflectHeading(Vector2 mtd);   /* the C# heading mirror formula */
	void CountBounce();
	void InitializeOne(EnemyDifficulty difficulty);

	/* EnemyObj_BouncySpike.cs line 16, 18 (RotationSpeed 250 is the
	 * visual spin, not ported: the blitter has no rotation). */
	float m_selfDestructTimer = 0.7f;
	int m_selfDestructCounter = 0;
	int m_selfDestructTotalBounces = 12;
	float m_internalOrientation = 0;
};

} // namespace RogueCastle

#endif
