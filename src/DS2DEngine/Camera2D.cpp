#include "Camera2D.h"
#include "../platform.h"

namespace DS2DEngine
{

/* Play area: 640x360 centered in the 640x480 frame (letterbox y = 60),
 * world drawn at Zoom 0.5 (AGENTS.md). */
static const float VIEW_CX = SCREEN_W * 0.5f;
static const float VIEW_CY = 60.0f + 180.0f;

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
	/* C# Camera2D.Bounds: centered on the position, world units. */
	int w = (int) (SCREEN_W / m_zoom);
	int h = (int) (360.0f / m_zoom);
	return Rectangle((int) (m_position.X - w * 0.5f), (int) (m_position.Y - h * 0.5f), w, h);
}

void Camera2D::Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
                   float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects, float layer)
{
	(void) layer;
	/* rotation is in radians, like XNA SpriteBatch */
	Vector2 screen((position.X - m_position.X) * m_zoom + VIEW_CX,
	               (position.Y - m_position.Y) * m_zoom + VIEW_CY);
	/* XNA: the camera zoom scales the whole draw, so a scale-2 object in a
	 * 0.5-zoom camera lands 1:1 on screen (AGENTS.md rendering rules). */
	Vector2 screenScale(scale.X * m_zoom, scale.Y * m_zoom);
	float dx = screen.X - origin.X * screenScale.X;
	float dy = screen.Y - origin.Y * screenScale.Y;
	float s = screenScale.X;
	/* XNA draw colour (TextureColor * Opacity); white costs nothing extra */
	uint32_t argb = ((uint32_t) color.A << 24) | ((uint32_t) color.R << 16) | ((uint32_t) color.G << 8) | color.B;
	if (rotation != 0.0f || screenScale.X != screenScale.Y)
	{
		gfx_blit_ex(texture, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, screen.X, screen.Y, origin.X, origin.Y,
		            screenScale.X, screenScale.Y, rotation, effects == SpriteEffects::FlipHorizontally ? 1 : 0, argb);
		return;
	}
	gfx_blit_tinted(texture, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height,
	                dx, dy, s, effects == SpriteEffects::FlipHorizontally ? 1 : 0, argb);
}

void Camera2D::Draw(const Texture *texture, Vector2 position, Rectangle srcRect, Color color,
                   float rotation, Vector2 origin, Vector2 scale, SpriteEffects effects)
{
	Draw(texture, position, srcRect, color, rotation, origin, scale, effects, 1);
}

void Camera2D::DrawRectangle(Vector2 position, int width, int height, uint32_t colour)
{
	/* The C# draws a 1x1 generic texture scaled up; the port fills the
	 * transformed rectangle directly. */
	float x0 = (position.X - m_position.X) * m_zoom + VIEW_CX;
	float y0 = (position.Y - m_position.Y) * m_zoom + VIEW_CY;
	int sx = (int) (x0 + (x0 >= 0.0f ? 0.5f : -0.5f));
	int sy = (int) (y0 + (y0 >= 0.0f ? 0.5f : -0.5f));
	int sw = (int) (width * m_zoom + 0.5f);
	int sh = (int) (height * m_zoom + 0.5f);
	gfx_fill(sx, sy, sw, sh, colour);
}

} // namespace DS2DEngine
