/* Port of RogueCastle/src/Game Objects/HoverObj.cs: a sprite that bobs
 * around its starting position forever - the garden's floating rocks
 * (LevelParser.cs line 361 turns the swapped GardenFloatingRock sprites
 * into these, Amplitude -50..50, HoverSpeed -2..2). Drawn live, never
 * baked into the room cache (GameObj::IsHovering).
 */
#ifndef ROGUE_CASTLE_HOVER_OBJ_H
#define ROGUE_CASTLE_HOVER_OBJ_H

#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../debugflags.h"

#include <cmath>
#include <cstdio>

namespace RogueCastle
{

class HoverObj : public DS2DEngine::SpriteObj
{
public:
	HoverObj(const std::string &spriteName, float amplitude, float hoverSpeed)
		: SpriteObj(spriteName), m_amplitude(amplitude), m_hoverSpeed(hoverSpeed)
	{
	}

	bool IsHovering() const override { return true; }

	/* HoverObj.cs line 24: Y = start + sin(TotalGameTime * speed) * amp.
	 * The C# reads the global game clock; the port accumulates the
	 * camera's elapsed time when drawn (SpriteObj.cs does the same for
	 * the animation), so an off-screen rock pauses - a harmless phase
	 * shift. */
	void Draw(DS2DEngine::Camera2D *camera) override
	{
		m_time += (float) camera->ElapsedTotalSeconds();
		Y(m_startY + sinf(m_time * m_hoverSpeed) * m_amplitude);
		if (rl_trace() && (int) m_time > m_traced)
		{
			m_traced = (int) m_time;
			std::printf("hover %s t=%.2f amp=%.1f speed=%.2f world (%.0f,%.0f) cam (%.0f,%.0f) screen (%.0f,%.0f)\n",
				SpriteName().c_str(), m_time, m_amplitude, m_hoverSpeed, X(), Y(), camera->X(), camera->Y(),
				(X() - camera->X()) * camera->Zoom() + 320,
				(Y() - camera->Y()) * camera->Zoom() + 240);
		}
		DS2DEngine::SpriteObj::Draw(camera);
	}

	void SetStartingPos(Vector2 pos)
	{
		Position(pos);
		m_startY = pos.Y;
	}

private:
	float m_startY = 0;
	float m_amplitude;
	float m_hoverSpeed;
	float m_time = 0;
	int m_traced = 0;
};

} // namespace RogueCastle

#endif
