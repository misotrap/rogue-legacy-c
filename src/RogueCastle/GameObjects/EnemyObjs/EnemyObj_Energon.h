/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Energon.cs:
 * the construct walks around and fires a weapon-ghost projectile
 * (sword/shield/downsword, switched at random) that homes at the hero.
 * The sword does not hurt it: the hero must send a shot back with the move
 * of its type (sword, down strike or block) while its shield matches.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_ENERGON_H
#define ROGUE_CASTLE_ENEMY_OBJ_ENERGON_H

#include "EnemyObj.h"
#include "../../../DS2DEngine/GameObjs/SpriteObj.h"

namespace RogueCastle
{

class EnemyObj_Energon : public EnemyObj
{
public:
	EnemyObj_Energon(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_Energon() override;
	void Draw(DS2DEngine::Camera2D *camera) override;
	void Kill(bool giveXP = true) override;
	void CollisionResponse(DS2DEngine::CollisionBox *thisBox, DS2DEngine::CollisionBox *otherBox,
	                       int collisionResponseType) override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	/* EnemyObj_Energon.cs lines 12-14. */
	static const unsigned char TYPE_SWORD = 0;
	static const unsigned char TYPE_SHIELD = 1;
	static const unsigned char TYPE_DOWNSWORD = 2;

	enum Action
	{
		ACTION_NONE,
		ACTION_WALK_TOWARDS,
		ACTION_WALK_AWAY,
		ACTION_WALK_STOP,
		ACTION_FIRE,
		ACTION_CHANGE_TYPE
	};

	void PickAction(const int *weights, int count, const Action *actions);
	void StartAction(Action action);
	void StartCooldownMove(int move) override;
	void UpdateAction();
	void FireProjectile();
	void SwitchRandomType();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	unsigned char m_currentAttackType = TYPE_SWORD;
	/* The see-through weapon over the body: the one its shots answer to */
	DS2DEngine::SpriteObj *m_shield = nullptr;
	void SwitchType(unsigned char type);

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}
};

} // namespace RogueCastle

#endif
