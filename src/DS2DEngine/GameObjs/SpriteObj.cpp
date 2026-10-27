/* Port of DS2DEngine/src/GameObjs/SpriteObj.cs. */
#include "SpriteObj.h"

namespace DS2DEngine
{

SpriteObj::SpriteObj(const std::string &spriteName)
{
	LoadSpriteData(spriteName);
}

SpriteObj::~SpriteObj()
{
}

void SpriteObj::LoadSpriteData(const std::string &spriteName)
{
	_spriteName = spriteName;
	m_imageDataList = SpriteSystem::SpriteLibrary::GetImageDataList(spriteName);
	m_frameDataList = SpriteSystem::SpriteLibrary::GetFrameDataList(spriteName);
	m_spritesheetName = SpriteSystem::SpriteLibrary::GetSpritesheetName(spriteName);

	_frameIndex = 0;
	_ssRect = Rectangle();
	if (m_imageDataList == nullptr || m_imageDataList->empty())
	{
		m_imageDataList = nullptr;
		m_frameDataList = nullptr;
		_frameCount = 0;
		return;
	}
	const SpriteSystem::ImageData &imageData = (*m_imageDataList)[0];
	_ssRect = Rectangle((int) imageData.SSPos.X, (int) imageData.SSPos.Y, imageData.Width, imageData.Height);
	AnchorX(imageData.Anchor.X);
	AnchorY(imageData.Anchor.Y);

	_frameCount = m_frameDataList != nullptr ? (int) m_frameDataList->size() : 0;
	if (_frameCount > 1)
	{
		const SpriteSystem::FrameData &fd = (*m_frameDataList)[_frameIndex];
		_frameDuration = fd.Duration * AnimationSpeed();
		_frameDelayCounter = 1;
	}
}

void SpriteObj::ChangeSprite(const std::string &spriteName)
{
	LoadSpriteData(spriteName);
	StopAnimation();
}

int SpriteObj::AnimationSpeed() const
{
	/* SpriteObj.cs: the parent container's speed is used when attached. */
	if (Parent() == nullptr)
		return m_hackFrameRate;
	return Parent()->AnimationSpeed();
}

float SpriteObj::AnimationDelay() const
{
	if (Parent() == nullptr || m_overrideParentAnimationDelay)
		return m_timeDelay;
	return Parent()->AnimationDelay();
}

int SpriteObj::TotalFrames() const
{
	int totalFrames = 0;
	if (m_frameDataList != nullptr)
	{
		for (const SpriteSystem::FrameData &fd : *m_frameDataList)
			totalFrames += fd.Duration * AnimationSpeed();
	}
	return totalFrames;
}

void SpriteObj::PlayAnimation(bool loopAnimation)
{
	_animate = true;
	_loop = loopAnimation;
	_startAnimationIndex = 1;
	_endAnimationIndex = TotalFrames();
	m_timeDelayCounter = 0;
	GoToFrame(_startAnimationIndex);
}

void SpriteObj::PlayAnimation(int startIndex, int endIndex, bool loopAnimation)
{
	startIndex = (startIndex - 1) * AnimationSpeed() + 1;
	endIndex = endIndex * AnimationSpeed();
	if (startIndex < 1)
		startIndex = 1;
	else if (startIndex > TotalFrames())
		startIndex = TotalFrames();
	if (endIndex < 1)
		endIndex = 1;
	else if (endIndex > TotalFrames())
		endIndex = TotalFrames();

	_animate = true;
	_loop = loopAnimation;
	_startAnimationIndex = startIndex;
	_endAnimationIndex = endIndex;
	m_timeDelayCounter = 0;
	GoToFrame(startIndex);
}

void SpriteObj::PlayAnimation(const std::string &startLabel, const std::string &endLabel, bool loopAnimation)
{
	int startIndex = FindLabelIndex(startLabel);
	int endIndex = FindLabelIndex(endLabel);
	if (startIndex != -1 && endIndex != -1)
		PlayAnimation(startIndex, endIndex, loopAnimation);
}

int SpriteObj::FindLabelIndex(const std::string &label) const
{
	if (m_frameDataList == nullptr)
		return -1;
	for (const SpriteSystem::FrameData &fd : *m_frameDataList)
	{
		if (fd.Label == label)
			return fd.Frame;
	}
	return -1;
}

void SpriteObj::StopAnimation()
{
	_animate = false;
	m_wasAnimating = false;
}

void SpriteObj::PauseAnimation()
{
	m_wasAnimating = _animate;
	_animate = false;
}

void SpriteObj::ResumeAnimation()
{
	if (m_wasAnimating)
		_animate = true;
	m_wasAnimating = false;
}

void SpriteObj::GoToFrame(int index)
{
	/* SpriteObj.cs: 'index' counts animation ticks (1-based); find the
	 * frame that contains that tick and show it. */
	if (m_frameDataList == nullptr || m_frameDataList->empty())
		return;
	int totalFrames = 0;
	int frameStart = 0;
	for (size_t i = 0; i < m_frameDataList->size(); i++)
	{
		const SpriteSystem::FrameData &fd = (*m_frameDataList)[i];
		totalFrames += fd.Duration * AnimationSpeed();
		if (totalFrames >= index)
		{
			frameStart = (int) i;
			break;
		}
	}
	_frameIndex = frameStart;
	_currentFrameIndex = index;
	const SpriteSystem::FrameData &fd2 = (*m_frameDataList)[_frameIndex];
	_frameDuration = fd2.Duration * AnimationSpeed();
	_frameDelayCounter = _frameDuration - (totalFrames - index);

	const SpriteSystem::ImageData &id = (*m_imageDataList)[_frameIndex];
	_ssRect.X = (int) id.SSPos.X;
	_ssRect.Y = (int) id.SSPos.Y;
	_ssRect.Width = id.Width;
	_ssRect.Height = id.Height;
	AnchorX(id.Anchor.X);
	AnchorY(id.Anchor.Y);
}

void SpriteObj::GoToNextFrame()
{
	/* Only animate if there is more than one frame in the animation. */
	if (_frameCount > 1)
	{
		if (_loop || _currentFrameIndex < _endAnimationIndex)
		{
			if (_frameDelayCounter < _frameDuration)
			{
				_frameDelayCounter++;
				_currentFrameIndex++;
			}
			else
			{
				_frameDelayCounter = 1;
				_frameIndex++;
				_currentFrameIndex++;

				if (_currentFrameIndex > _endAnimationIndex)
					GoToFrame(_startAnimationIndex);

				if (_frameIndex >= _frameCount)
					_frameIndex = 0;

				const SpriteSystem::FrameData &fd = (*m_frameDataList)[_frameIndex];
				_frameDuration = fd.Duration * AnimationSpeed();
				const SpriteSystem::ImageData &id = (*m_imageDataList)[_frameIndex];
				_ssRect.X = (int) id.SSPos.X;
				_ssRect.Y = (int) id.SSPos.Y;
				_ssRect.Width = id.Width;
				_ssRect.Height = id.Height;
				AnchorX(id.Anchor.X);
				AnchorY(id.Anchor.Y);
			}
		}
	}
}

/* XNA draw colour: TextureColor * Opacity (premultiplied) */
static ::Color ColourOf(const ::Color &c, float opacity)
{
	float a = opacity < 0 ? 0 : (opacity > 1 ? 1 : opacity);
	return ::Color((unsigned) (c.R * a + 0.5f), (unsigned) (c.G * a + 0.5f), (unsigned) (c.B * a + 0.5f), (unsigned) (c.A * a + 0.5f));
}

void SpriteObj::Draw(Camera2D *camera)
{
	if (!Visible())
		return;
	const Texture *sprite = SpriteSystem::SpriteLibrary::GetSprite(_spriteName);
	if (sprite == nullptr)
		return;

	if (Parent() == nullptr)
	{
		/* Camera culling for standalone sprites; children rely on the
		 * parent's test (SpriteObj.cs). */
		if (ForceDraw() || Bounds().Intersects(camera->Bounds()))
			camera->Draw(sprite, AbsPosition(), _ssRect, ColourOf(TextureColor(), Opacity()),
			             0, Anchor(), Scale(), Flip(), Layer());
	}
	else
	{
		camera->Draw(sprite, AbsPosition(), _ssRect, ColourOf(TextureColor(), Opacity()),
		             0, Anchor(), Parent()->Scale() * Scale(), Flip(), Layer());
	}

	/* Animation advances in Draw (SpriteObj.cs), once per drawn frame. */
	double elapsedTotalSeconds = camera->ElapsedTotalSeconds();
	if (_animate && m_totalGameTime != elapsedTotalSeconds)
	{
		m_totalGameTime = elapsedTotalSeconds;
		m_timeDelayCounter += (float) elapsedTotalSeconds;
		if (_frameCount > 1 && m_timeDelayCounter >= AnimationDelay())
		{
			m_timeDelayCounter = 0;
			if (_currentFrameIndex < _endAnimationIndex || _loop)
				GoToNextFrame();
			else
			{
				if (!m_atEndFrame)
					m_atEndFrame = true;
				else
				{
					m_atEndFrame = false;
					_animate = false;
				}
			}
		}
	}
}

} // namespace DS2DEngine
