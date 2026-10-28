/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_Portrait.cs:
 * the haunted painting shakes when the player lingers near it, and
 * flies after them when they walk away (the original's weight tables do
 * exactly that). The advanced, expert and miniboss chases open with a
 * ghost projectile spread. The visual shake offset and the miniboss's
 * random inner picture are not ported.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_PORTRAIT_H
#define ROGUE_CASTLE_ENEMY_OBJ_PORTRAIT_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_Portrait : public EnemyObj
{
public:
	EnemyObj_Portrait(PlayerObj *target, EnemyDifficulty difficulty);
	~EnemyObj_Portrait() override;
	void HitEnemy(int damage, Vector2 collisionPt, bool isPlayer) override;
	void Draw(DS2DEngine::Camera2D *camera) override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override;
	void RunAdvancedLogic() override { RunBasicLogic(); }
	void RunExpertLogic() override { RunBasicLogic(); }
	void RunMinibossLogic() override { RunBasicLogic(); }
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum Action
	{
		ACTION_NONE,
		ACTION_SHAKE,
		ACTION_CHASE
	};

	void StartAction(Action action);
	void UpdateAction();
	void FireSpread();
	void InitializeOne(EnemyDifficulty difficulty);

	Action m_action = ACTION_NONE;
	int m_endPhase = 0;
	bool m_shake = false;     // Shake: the warning wobble
	bool m_chasing = false;   // Chasing: awake, spinning after the hero
	bool m_giant = false;
	float m_time = 0;
	std::string m_picture;   /* the painting in the frame */
	DS2DEngine::SpriteObj *m_face = nullptr;   /* the painting, then the true face */
	void ChasePlayer();

	bool ActionComplete() override
	{
		return m_actionPhase >= m_endPhase && m_actionTimer <= 0;
	}
};

} // namespace RogueCastle

#endif
