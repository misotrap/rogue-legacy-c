/* Port of DS2DEngine/src/GameObjs/SpriteObj.cs: a single sprite with its
 * animation (frame durations counted in AnimationDelay ticks) and drawing.
 *
 * Frame indices follow the C# conventions: CurrentFrame and the GoToFrame
 * argument are 1-based in "tick" units (TotalFrames = sum of durations).
 */
#ifndef DS2D_SPRITE_OBJ_H
#define DS2D_SPRITE_OBJ_H

#include "GameObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include <string>
#include <vector>

namespace DS2DEngine
{

class SpriteObj : public GameObj
{
public:
	explicit SpriteObj(const std::string &spriteName);
	~SpriteObj() override;

	void ChangeSprite(const std::string &spriteName) override;
	void Draw(Camera2D *camera) override;

	/* --- animation (SpriteObj.cs) --- */
	void PlayAnimation(bool loopAnimation = true) override;
	void PlayAnimation(int startIndex, int endIndex, bool loopAnimation = false) override;
	void PlayAnimation(const std::string &startLabel, const std::string &endLabel, bool loopAnimation = false) override;
	void StopAnimation() override;
	void PauseAnimation() override;
	void ResumeAnimation() override;
	void GoToNextFrame() override;
	void GoToFrame(int index) override;
	int FindLabelIndex(const std::string &label) const override;
	bool IsAnimating() const override { return _animate; }
	bool IsLooping() const override { return _loop; }
	int CurrentFrame() const override { return _frameIndex; }
	int TotalFrames() const override;
	void AnimationSpeed(int value) { m_hackFrameRate = value; }
	int AnimationSpeed() const override;
	float AnimationDelay() const override;
	void AnimationDelay(float value) { m_timeDelay = value; }
	void OverrideParentAnimationDelay(bool value) { m_overrideParentAnimationDelay = value; }

	/* SpriteObj.cs overrides Width/Height with the source rectangle. */
	float Width() const override { return _ssRect.Width * ScaleX(); }
	float Height() const override { return _ssRect.Height * ScaleY(); }

	const std::string &SpritesheetName() const { return m_spritesheetName; }
	const Rectangle &SpriteRect() const { return _ssRect; }

protected:
	const std::vector<SpriteSystem::ImageData> *m_imageDataList = nullptr;
	const std::vector<SpriteSystem::FrameData> *m_frameDataList = nullptr;
	Rectangle _ssRect;
	std::string m_spritesheetName;
	bool _animate = false;
	bool _loop = true;
	bool m_wasAnimating = false;
	bool m_overrideParentAnimationDelay = false;
	int _frameIndex = 0;          /* 0-based position in the frame list */
	int _frameCount = 0;          /* number of frames in the list */
	int _currentFrameIndex = 1;   /* tick-aware index (SpriteObj.cs) */
	int _startAnimationIndex = 0;
	int _endAnimationIndex = 0;
	int _frameDuration = 1;       /* of the current frame, in ticks */
	int _frameDelayCounter = 1;
	bool m_atEndFrame = false;
	float m_timeDelay = 0;
	float m_timeDelayCounter = 0;
	double m_totalGameTime = 0;
	int m_hackFrameRate = 1;

	void LoadSpriteData(const std::string &spriteName);
};

} // namespace DS2DEngine

#endif
