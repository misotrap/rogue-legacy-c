/* Port of DS2DEngine/src/GameObjs/GameObj.cs. */
#include "GameObj.h"

namespace DS2DEngine
{

GameObj::GameObj()
	: _position(0, 0), _scale(1, 1), _anchor(0, 0), _heading(0, 0),
	  _width(0), _height(0), _rotation(0), _opacity(1), _layer(0), _speed(0),
	  _flip(SpriteEffects::None), _parent(nullptr),
	  _visible(true), _forceDraw(false), _addToBounds(true),
	  _useCachedValues(false)
{
}

GameObj::~GameObj()
{
}

/* --- X (GameObj.cs: flip mirrors the child offset around the parent) --- */
float GameObj::X() const
{
	if (_parent == nullptr || Flip() != SpriteEffects::FlipHorizontally)
		return _position.X;
	return -_position.X * _parent->ScaleX();   // otherwise flip it horizontally
}

void GameObj::X(float value)
{
	_position.X = value;
}

float GameObj::Y() const
{
	return _position.Y;
}

void GameObj::Y(float value)
{
	_position.Y = value;
}

Vector2 GameObj::Position() const
{
	if (_parent == nullptr)
		return _position;
	if (Flip() == SpriteEffects::FlipHorizontally)
		return Vector2(-_position.X * _parent->ScaleX(), _position.Y * _parent->ScaleY());
	return _position * _parent->Scale();
}

void GameObj::Position(const Vector2 &value)
{
	_position = value;
}

SpriteEffects GameObj::Flip() const
{
	/* C#: delegates to the topmost parent's flip. */
	GameObj *top = TopmostParent();
	if (top == nullptr || top == this)
		return _flip;
	return top->_flip;
}

void GameObj::Flip(SpriteEffects value)
{
	_flip = value;
}

GameObj *GameObj::TopmostParent() const
{
	GameObj *obj = _parent;
	while (obj != nullptr && obj->_parent != nullptr)
		obj = obj->_parent;
	return obj;
}

float GameObj::AbsX() const
{
	if (_parent == nullptr)
		return X();
	return _parent->X() + RotatedPoint().X;
}

float GameObj::AbsY() const
{
	if (_parent == nullptr)
		return (int) Y();   // casting to int: too fine a Y value jitters
	return (int) _parent->Y() + (int) RotatedPoint().Y;
}

Vector2 GameObj::AbsPosition() const
{
	return Vector2(AbsX(), AbsY());
}

Vector2 GameObj::RotatedPoint() const
{
	/* Parent rotation is not applied anywhere in the ported game yet. */
	return Position();
}

/* Anchor with flip: the anchor becomes the mirrored point (GameObj.cs). */
float GameObj::AnchorX() const
{
	if (Flip() == SpriteEffects::FlipHorizontally)
		return Width() / ScaleX() - _anchor.X;
	return _anchor.X;
}

float GameObj::AnchorY() const
{
	return _anchor.Y;
}

Vector2 GameObj::Anchor() const
{
	if (Flip() == SpriteEffects::FlipHorizontally)
		return Vector2(Width() / ScaleX() - _anchor.X, _anchor.Y);
	return _anchor;
}

/* Bounds (GameObj.cs): Width()/Height() already include the object's own
 * scale, so only the parent's is applied. */
Rectangle GameObj::Bounds() const
{
	if (_parent == nullptr)
	{
		if (Rotation() == 0)
			return Rectangle((int) (-AnchorX() * ScaleX() + X()),
			                 (int) (-AnchorY() * ScaleY() + Y()),
			                 (int) Width(), (int) Height());
		/* TODO C++: CollisionMath.RotatedRectBounds for rotated objects. */
		return Rectangle((int) (-AnchorX() * ScaleX() + X()),
		                 (int) (-AnchorY() * ScaleY() + Y()),
		                 (int) Width(), (int) Height());
	}
	return Rectangle((int) ((-AnchorX() * ScaleX() + X()) * _parent->ScaleX()),
	                 (int) ((-AnchorY() * ScaleY() + Y()) * _parent->ScaleY()),
	                 (int) (Width() * _parent->ScaleX()),
	                 (int) (Height() * _parent->ScaleY()));
}

} // namespace DS2DEngine
