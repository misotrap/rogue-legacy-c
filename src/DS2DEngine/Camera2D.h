/* Port of DS2DEngine/src/Camera2D.cs (only what the port needs).
 * C# Camera2D extends SpriteBatch; here Draw() converts world coordinates
 * to the 640x480 frame through the camera transform and calls gfx_blit.
 *
 * The camera POSITION is the CENTER of the visible rectangle (like C#
 * Camera2D, see UpdateCamera in ProceduralLevelScreen.cs):
 *
 *   screen = (world - position) * Zoom + (320, 240)
 *
 * The game world is 1320x720 (GlobalEV) drawn at Zoom 0.5 into the 640x480
 * frame, letterboxed at y = 60 (AGENTS.md rendering rules).
 */
#ifndef DS2D_CAMERA2D_H
#define DS2D_CAMERA2D_H

#include "Types.h"
#include "../gfx.h"

namespace DS2DEngine
{

class Camera2D
{
public:
	Camera2D();

	/* SpriteBatch-equivalent Draw calls used by the game objects. */
	void Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
	          float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects, float layer);
	void Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
	          float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects);

	/* camera.Draw(GenericTexture, position, rect, colour, ...) for plain
	 * coloured rectangles (walls, doorways). */
	void DrawRectangle(Vector2 position, int width, int height, uint32_t colour);

	/* C# Camera2D position (world units, CENTER of the view). */
	float X() const { return m_position.X; }
	void X(float value) { m_position.X = value; }
	float Y() const { return m_position.Y; }
	void Y(float value) { m_position.Y = value; }
	Vector2 Position() const { return m_position; }
	void Position(const Vector2 &value) { m_position = value; }

	float Zoom() const { return m_zoom; }
	void Zoom(float value);
	/* World rectangle currently visible (C# Camera2D.Bounds). */
	Rectangle Bounds() const;

	/* The C# animation code reads the elapsed time off the camera. */
	double ElapsedTotalSeconds() const { return m_elapsedTotalSeconds; }
	void ElapsedTotalSeconds(double value) { m_elapsedTotalSeconds = value; }
	/* C# Camera2D.GameTime property ('GameTime' clashes with the type on
	 * GCC, which is why the getter has this name). */
	GameTime *GetGameTime() { return &m_gameTime; }

private:
	Vector2 m_position;
	float m_zoom;
	double m_elapsedTotalSeconds;
	GameTime m_gameTime;
};

} // namespace DS2DEngine

#endif
