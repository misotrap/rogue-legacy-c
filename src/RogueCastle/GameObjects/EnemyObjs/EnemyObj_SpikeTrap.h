/* Port of RogueCastle/src/Game Objects/EnemyObjs/EnemyObj_SpikeTrap.cs:
 * the floor spikes extract when the player steps on them (a 120x30
 * detection rect above the trap), hurt through their weapon hitboxes
 * (the sprite's R records), and retract when the player leaves. The
 * original drives the frames with one LogicSet; the port keeps the same
 * frame sequence (1-2, pause, 2-4 extract; 5-7 retract) in a small state
 * machine, since the C# frame-index checks are 1-based there.
 */
#ifndef ROGUE_CASTLE_ENEMY_OBJ_SPIKE_TRAP_H
#define ROGUE_CASTLE_ENEMY_OBJ_SPIKE_TRAP_H

#include "EnemyObj.h"

namespace RogueCastle
{

class EnemyObj_SpikeTrap : public EnemyObj
{
public:
	EnemyObj_SpikeTrap(PlayerObj *target, EnemyDifficulty difficulty);
	void ResetState() override;

protected:
	void InitializeEV() override;
	void RunBasicLogic() override {}
	void RunAdvancedLogic() override {}
	void RunExpertLogic() override {}
	void RunMinibossLogic() override {}
	void UpdateEnemy(GameTime *gameTime) override;

private:
	enum TrapState
	{
		TRAP_HIDDEN,       /* frame 1 */
		TRAP_EXTRACTING,   /* m_extractLS: frames 1-2, 0.1 s, 2-4 */
		TRAP_EXTRACTED,    /* resting on the last extract frame */
		TRAP_RETRACTING    /* frames 5-7 */
	};

	TrapState m_trapState = TRAP_HIDDEN;
	float ExtractDelay = 0.1f;   /* EnemyObj_SpikeTrap.cs line 16 */
	/* DetectionRect (line 24): the player standing on the trap. */
	int m_detectWidth = 120;
	int m_detectHeight = 30;
};

} // namespace RogueCastle

#endif
