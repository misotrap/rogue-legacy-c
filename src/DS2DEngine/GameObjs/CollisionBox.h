/* Port of DS2DEngine/src/GameObjs/CollisionBox.cs: a hitbox attached to a
 * game object, positioned relative to the object's origin (anchor point).
 * Only the axis-aligned paths are ported (rotation is stored but unused).
 */
#ifndef DS2D_COLLISION_BOX_H
#define DS2D_COLLISION_BOX_H

#include "GameObj.h"
#include <cstdint>

namespace DS2DEngine
{

class GameObj;

class CollisionBox
{
public:
	CollisionBox(GameObj *parent);
	CollisionBox(int x, int y, int width, int height, int type, GameObj *parent);

	int X() const;
	void X(int value) { _rectangle.X = value; }
	int Y() const { return _rectangle.Y; }
	void Y(int value) { _rectangle.Y = value; }
	int Width() const { return _rectangle.Width; }
	void Width(int value) { _rectangle.Width = value; }
	int Height() const { return _rectangle.Height; }
	void Height(int value) { _rectangle.Height = value; }
	int Type() const { return _collisionType; }
	void Type(int value) { _collisionType = value; }
	bool WasAdded = false;

	/* Rotation in degrees, as in C# (InternalRotation handles the sign). */
	float InternalRotation() const;
	void InternalRotation(float degrees) { m_rotationDegrees = degrees; }
	float AbsRotation() const;

	int AbsX() const;
	int AbsY() const;
	Rectangle AbsRect() const { return Rectangle(AbsX(), AbsY(), Width(), Height()); }
	Rectangle NonRotatedAbsRect() const { return Rectangle(X() + m_parentObj->AbsX(), Y() + m_parentObj->AbsY(), Width(), Height()); }

	GameObj *Parent() const { return m_parentObj; }

private:
	Rectangle _rectangle;
	GameObj *m_parentObj;
	int _collisionType;
	float m_rotationDegrees = 0;
};

} // namespace DS2DEngine

#endif
