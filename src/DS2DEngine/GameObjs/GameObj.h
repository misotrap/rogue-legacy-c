/* Port of DS2DEngine/src/GameObjs/GameObj.cs: base object with position,
 * scale, flip, anchor, parent/child relations and the position math the
 * whole engine relies on (AbsPosition, flip-adjusted anchors, bounds).
 *
 * C# properties become a getter/setter pair with the same name:
 * this.X -> obj.X(), obj.X = 5 -> obj.X(5).
 * Getters must not be overridden except where C# marks them virtual.
 */
#ifndef DS2D_GAME_OBJ_H
#define DS2D_GAME_OBJ_H

#include "../Camera2D.h"
#include "../Types.h"
#include <string>

namespace DS2DEngine
{

class ObjContainer;
class PhysicsObj;
class PhysicsObjContainer;

class GameObj
{
public:
	GameObj();
	virtual ~GameObj();

	virtual void Update(GameTime *gameTime) { (void) gameTime; }
	virtual void Draw(Camera2D *camera) { (void) camera; }
	virtual void DrawOutline(Camera2D *camera) { (void) camera; }
	virtual void ChangeSprite(const std::string &spriteName) { (void) spriteName; }

	/* --- position / scale / flip (GameObj.cs) --- */
	float X() const;
	void X(float value);
	float Y() const;
	void Y(float value);
	Vector2 Position() const;
	void Position(const Vector2 &value);

	float ScaleX() const { return _scale.X; }
	virtual void ScaleX(float value) { _scale.X = value; }
	float ScaleY() const { return _scale.Y; }
	virtual void ScaleY(float value) { _scale.Y = value; }
	Vector2 Scale() const { return _scale; }
	virtual void Scale(const Vector2 &value) { _scale = value; }

	float Rotation() const { return _rotation; }
	void Rotation(float value) { _rotation = value; }

	virtual SpriteEffects Flip() const;
	virtual void Flip(SpriteEffects value);

	float AnchorX() const;
	virtual void AnchorX(float value) { _anchor.X = value; }
	float AnchorY() const;
	virtual void AnchorY(float value) { _anchor.Y = value; }
	virtual Vector2 Anchor() const;
	void Anchor(const Vector2 &value) { _anchor = value; }

	/* --- tree --- */
	GameObj *Parent() const { return _parent; }
	void Parent(GameObj *value) { _parent = value; }
	GameObj *TopmostParent() const;   /* C# _topmostParent, computed by walking up */

	float AbsX() const;
	float AbsY() const;
	Vector2 AbsPosition() const;

	/* --- common flags --- */
	bool Visible() const { return _visible; }
	void Visible(bool value) { _visible = value; }
	bool ForceDraw() const { return _forceDraw; }
	void ForceDraw(bool value) { _forceDraw = value; }
	float Opacity() const { return _opacity; }
	void Opacity(float value) { _opacity = value; }
	float Layer() const { return _layer; }
	void Layer(float value) { _layer = value; }
	bool AddToBounds() const { return _addToBounds; }
	void AddToBounds(bool value) { _addToBounds = value; }
	bool UseCachedValues() const { return _useCachedValues; }
	void UseCachedValues(bool value) { _useCachedValues = value; }
	float HeadingX() const { return _heading.X; }
	void HeadingX(float value) { _heading.X = value; }
	float HeadingY() const { return _heading.Y; }
	void HeadingY(float value) { _heading.Y = value; }
	Vector2 Heading() const { return _heading; }
	void Heading(const Vector2 &value) { _heading = value; }
	float CurrentSpeed() const { return _speed; }
	void CurrentSpeed(float value) { _speed = value; }
	std::string SpriteName() const { return _spriteName; }

	/* C# 'as' casts, emulated without RTTI: each class overrides its own. */
	virtual ObjContainer *AsObjContainer() { return nullptr; }
	virtual PhysicsObj *AsPhysicsObj() { return nullptr; }
	virtual PhysicsObjContainer *AsPhysicsObjContainer() { return nullptr; }

	/* Bounds: the axis-aligned rectangle around the object, in absolute
	 * (parent-applied) space. Only Rotation == 0 is ported so far. */
	virtual Rectangle Bounds() const;

	/* IAnimateableObj: containers override these so their children inherit
	 * the container's animation timing (SpriteObj.cs AnimationDelay). */
	virtual float AnimationDelay() const { return 0; }
	virtual int AnimationSpeed() const { return 1; }

	/* IAnimateableObj surface (C# interface, emulated with virtuals so the
	 * port can build without RTTI). */
	virtual void PlayAnimation(bool loopAnimation = true) { (void) loopAnimation; }
	virtual void PlayAnimation(int startIndex, int endIndex, bool loopAnimation = false)
	{ (void) startIndex; (void) endIndex; (void) loopAnimation; }
	virtual void PlayAnimation(const std::string &startLabel, const std::string &endLabel, bool loopAnimation = false)
	{ (void) startLabel; (void) endLabel; (void) loopAnimation; }
	virtual void StopAnimation() {}
	virtual void PauseAnimation() {}
	virtual void ResumeAnimation() {}
	virtual void GoToNextFrame() {}
	virtual void GoToFrame(int index) { (void) index; }
	virtual int FindLabelIndex(const std::string &label) const { (void) label; return -1; }
	virtual int TotalFrames() const { return 0; }
	virtual bool IsAnimating() const { return false; }
	virtual bool IsLooping() const { return false; }
	virtual int CurrentFrame() const { return 0; }

	/* Width/Height include the object's own scale (GameObj.cs). */
	virtual float Width() const { return _width * ScaleX(); }
	virtual float Height() const { return _height * ScaleY(); }

protected:
	Vector2 _position;
	Vector2 _scale;
	Vector2 _anchor;
	Vector2 _heading;
	float _width;
	float _height;
	float _rotation;
	float _opacity;
	float _layer;
	float _speed;             /* CurrentSpeed in C# */
	SpriteEffects _flip;
	GameObj *_parent;
	bool _visible;
	bool _forceDraw;
	bool _addToBounds;
	bool _useCachedValues;
	std::string _spriteName;

private:
	Vector2 RotatedPoint() const;   /* GameObj.cs: parent rotation applied */
};

} // namespace DS2DEngine

#endif
