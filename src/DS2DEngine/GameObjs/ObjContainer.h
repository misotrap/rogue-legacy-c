/* Port of DS2DEngine/src/GameObjs/ObjContainer.cs: a multi-part object
 * built from CharacterData (ChangeSprite swaps the child sprites).
 * The container owns and deletes its children.
 */
#ifndef DS2D_OBJ_CONTAINER_H
#define DS2D_OBJ_CONTAINER_H

#include "SpriteObj.h"
#include <string>
#include <vector>

namespace DS2DEngine
{

class ObjContainer : public GameObj
{
public:
	ObjContainer();
	explicit ObjContainer(const std::string &spriteName);
	~ObjContainer() override;

	void ChangeSprite(const std::string &spriteName) override;
	void Draw(Camera2D *camera) override;
	void Update(GameTime *gameTime) override;

	void AddChild(GameObj *obj);
	void AddChildAt(int index, GameObj *obj);
	void RemoveChild(GameObj *obj);
	void RemoveAll();
	GameObj *GetChildAt(int index) const;
	int NumChildren() const { return (int) m_objectList.size(); }

	/* Animation: forwarded to the children (ObjContainer.cs). */
	void PlayAnimation(bool loopAnimation = true) override;
	void PlayAnimation(int startIndex, int endIndex, bool loopAnimation = false) override;
	void PlayAnimation(const std::string &startLabel, const std::string &endLabel, bool loopAnimation = false) override;
	void StopAnimation() override;
	void PauseAnimation() override;
	void ResumeAnimation() override;
	void GoToNextFrame() override;
	void GoToFrame(int index) override;
	int FindLabelIndex(const std::string &label) const override;
	int TotalFrames() const override;   /* largest child animation */
	bool IsAnimating() const override { return m_animate; }
	bool IsLooping() const override { return m_loop; }
	int CurrentFrame() const override { return m_currentFrameIndex; }
	float AnimationDelay() const override { return AnimationDelayValue; }
	void AnimationDelay(float value) { AnimationDelayValue = value; }
	int AnimationSpeed() const override { return m_animationSpeed; }
	void AnimationSpeed(int value) { m_animationSpeed = value; }

	void CalculateBounds();
	Rectangle Bounds() const override { return m_bounds; }
	ObjContainer *AsObjContainer() override { return this; }

	float AnimationDelayValue = 0;

protected:
	std::vector<GameObj *> m_objectList;
	Rectangle m_bounds;
	int m_animationSpeed = 1;
	bool m_animate = false;
	bool m_loop = false;
	bool m_wasAnimating = false;
	int m_currentFrameIndex = 0;
	int m_startAnimationIndex = 0;
	int m_endAnimationIndex = 0;
	int m_numCharData = 0;        /* children added by ChangeSprite */
	int m_charDataStartIndex = -1;
	double m_totalGameTime = 0;
	float m_timeDelayCounter = 0;

	virtual void BuildFromCharData(const std::string &spriteName);
};

} // namespace DS2DEngine

#endif
