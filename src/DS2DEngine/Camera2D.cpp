#include "Camera2D.h"
#include "../platform.h"

namespace DS2DEngine
{

/* The 660x360 play area is letterboxed at y = 60 in the 640x480 frame and
 * the world is drawn at Zoom 0.5 (AGENTS.md). */
static const float VIEW_Y = 60.0f;

Camera2D::Camera2D()
	: m_position(0, 0), m_zoom(0.5f), m_elapsedTotalSeconds(0)
{
}

void Camera2D::Zoom(float value)
{
	if (value < 0.025f)
		value = 0.025f;
	m_zoom = value;
}

Rectangle Camera2D::Bounds() const
{
	/* World rectangle currently visible in the letterboxed area. */
	return Rectangle((int) m_position.X, (int) m_position.Y,
	                 (int) (SCREEN_W / m_zoom), (int) (360.0f / m_zoom));
}

void Camera2D::Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
                   float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects, float layer)
{
	(void) color;
	(void) layer;
	(void) rotation;   /* TODO C++: no rotation in the blitter yet; player
	                   * parts never rotate (the original ignores
	                   * childRotation), so nothing is lost so far. */
	Vector2 screen((position.X - m_position.X) * m_zoom, (position.Y - m_position.Y) * m_zoom + VIEW_Y);
	/* XNA: the camera zoom scales the whole draw, so a scale-2 object in a
	 * 0.5-zoom camera lands 1:1 on screen (AGENTS.md rendering rules). */
	Vector2 screenScale(scale.X * m_zoom, scale.Y * m_zoom);
	float dx = screen.X - origin.X * screenScale.X;
	float dy = screen.Y - origin.Y * screenScale.Y;
	float s = screenScale.X;
	gfx_blit(texture, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height,
	         dx, dy, s, effects == SpriteEffects::FlipHorizontally ? 1 : 0);
}

void Camera2D::Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
                   float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects)
{
	Draw(texture, position, srcRect, color, rotation, origin, scale, effects, 1);
}

} // namespace DS2DEngine
