/* XNA value types used by the ported C# code (Microsoft.Xna.Framework).
 * Kept in the global namespace so ported lines read the same: Vector2 v(x, y).
 */
#ifndef DS2D_TYPES_H
#define DS2D_TYPES_H

#include <cstdint>

struct Vector2
{
	float X, Y;
	Vector2() : X(0), Y(0) {}
	Vector2(float x, float y) : X(x), Y(y) {}
	static const Vector2 Zero;
	static const Vector2 One;
};

inline Vector2 operator+(const Vector2 &a, const Vector2 &b) { return Vector2(a.X + b.X, a.Y + b.Y); }
inline Vector2 operator-(const Vector2 &a, const Vector2 &b) { return Vector2(a.X - b.X, a.Y - b.Y); }
inline Vector2 operator*(const Vector2 &a, float s) { return Vector2(a.X * s, a.Y * s); }
inline Vector2 operator*(const Vector2 &a, const Vector2 &b) { return Vector2(a.X * b.X, a.Y * b.Y); }
inline Vector2 operator*(float s, const Vector2 &a) { return a * s; }
inline Vector2 operator-(const Vector2 &a) { return Vector2(-a.X, -a.Y); }
inline bool operator==(const Vector2 &a, const Vector2 &b) { return a.X == b.X && a.Y == b.Y; }
inline bool operator!=(const Vector2 &a, const Vector2 &b) { return !(a == b); }

struct Vector3
{
	float X, Y, Z;
	Vector3() : X(0), Y(0), Z(0) {}
	Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
};

struct Rectangle
{
	int X, Y, Width, Height;
	Rectangle() : X(0), Y(0), Width(0), Height(0) {}
	Rectangle(int x, int y, int w, int h) : X(x), Y(y), Width(w), Height(h) {}
	int Left() const { return X; }
	int Right() const { return X + Width; }
	int Top() const { return Y; }
	int Bottom() const { return Y + Height; }
	bool Intersects(const Rectangle &r) const
	{
		return Right() > r.Left() && Left() < r.Right() && Bottom() > r.Top() && Top() < r.Bottom();
	}
};

struct Color
{
	uint8_t R, G, B, A;
	Color() : R(0), G(0), B(0), A(0) {}
	Color(unsigned r, unsigned g, unsigned b) : R((uint8_t) r), G((uint8_t) g), B((uint8_t) b), A(255) {}
	Color(unsigned r, unsigned g, unsigned b, unsigned a)
		: R((uint8_t) r), G((uint8_t) g), B((uint8_t) b), A((uint8_t) a) {}
	static const Color White;
	static const Color Black;
};

enum class SpriteEffects
{
	None,
	FlipHorizontally,
	FlipVertically
};

/* The C# code passes GameTime around; the port only needs the fixed step. */
struct GameTime
{
	double ElapsedGameTime;   /* seconds (fixed 1/60) */
	GameTime() : ElapsedGameTime(0) {}
	explicit GameTime(double elapsed) : ElapsedGameTime(elapsed) {}
};

#endif
