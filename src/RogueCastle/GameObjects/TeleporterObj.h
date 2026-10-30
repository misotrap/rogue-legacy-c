/* Port of RogueCastle/src/Game Objects/TeleporterObj.cs: the pad the player
 * stands on to travel between the castle's teleporter rooms (one per Linker,
 * EntranceBoss and the CastleEntrance, LevelBuilder2.cs lines 186-203).
 * The floating arrow (UpArrowSquare_Sprite) shows while the hero stands on
 * it; Up opens the teleporter map (ProceduralLevelScreen polls the request).
 * Not a solid: SetCollision(false) in the C#, so no physics box here, the
 * bounds check in ProceduralLevelScreen::Update does it.
 */
#ifndef ROGUE_CASTLE_TELEPORTER_OBJ_H
#define ROGUE_CASTLE_TELEPORTER_OBJ_H

#include "../../DS2DEngine/GameObjs/SpriteObj.h"

#include <cmath>

namespace RogueCastle
{

class TeleporterObj : public DS2DEngine::SpriteObj
{
public:
	TeleporterObj()
		: SpriteObj("TeleporterBase_Sprite"),
		  m_arrow(new DS2DEngine::SpriteObj("UpArrowSquare_Sprite"))
	{
	}

	~TeleporterObj() { delete m_arrow; }

	/* Set by ProceduralLevelScreen::Update while the hero stands on the
	 * pad; Draw shows the arrow once, then hides it (CollisionResponse
	 * sets it again every frame in the C#). */
	bool ArrowVisible = false;

	/* TeleporterObj.cs line 27: the arrow floats 50 above the pad's top,
	 * swaying with the game clock. */
	void Draw(DS2DEngine::Camera2D *camera) override
	{
		m_time += (float) camera->ElapsedTotalSeconds();
		if (ArrowVisible)
		{
			Rectangle b = Bounds();
			m_arrow->Position(Vector2(b.X + b.Width * 0.5f,
				b.Y - 50 + sinf(m_time * 20) * 2));
			m_arrow->Draw(camera);
			ArrowVisible = false;
		}
		DS2DEngine::SpriteObj::Draw(camera);
	}

private:
	DS2DEngine::SpriteObj *m_arrow;
	float m_time = 0;
};

} // namespace RogueCastle

#endif
