/* Port of DS2DEngine/src/GameObjs/CollisionBox.cs. */
#include "CollisionBox.h"
#include "GameObj.h"
#include <cmath>

namespace DS2DEngine
{

CollisionBox::CollisionBox(GameObj *parent)
	: m_parentObj(parent), _collisionType(0)
{
}

CollisionBox::CollisionBox(int x, int y, int width, int height, int type, GameObj *parent)
	: _rectangle(x, y, width, height), m_parentObj(parent), _collisionType(type)
{
}

int CollisionBox::X() const
{
	/* CollisionBox.cs: flipping mirrors the box offset. */
	if (m_parentObj->Flip() == SpriteEffects::FlipHorizontally)
		return (int) -(_rectangle.X + _rectangle.Width);
	return _rectangle.X;
}

float CollisionBox::InternalRotation() const
{
	/* C# negates the rotation when the parent is flipped. */
	if (m_parentObj->Flip() == SpriteEffects::FlipHorizontally)
		return -m_rotationDegrees;
	return m_rotationDegrees;
}

float CollisionBox::AbsRotation() const
{
	return m_parentObj->Rotation() + InternalRotation();
}

int CollisionBox::AbsX() const
{
	return X() + (int) m_parentObj->AbsX();
}

int CollisionBox::AbsY() const
{
	return Y() + (int) m_parentObj->AbsY();
}

} // namespace DS2DEngine
