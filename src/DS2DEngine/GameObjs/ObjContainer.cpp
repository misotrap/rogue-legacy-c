/* Port of DS2DEngine/src/GameObjs/ObjContainer.cs. */
#include "ObjContainer.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include <climits>

namespace DS2DEngine
{

ObjContainer::ObjContainer()
{
	m_charDataStartIndex = -1;
}

ObjContainer::ObjContainer(const std::string &spriteName)
{
	m_charDataStartIndex = -1;
	_spriteName = spriteName;
	BuildFromCharData(spriteName);
}

ObjContainer::~ObjContainer()
{
	RemoveAll();
}

void ObjContainer::BuildFromCharData(const std::string &spriteName)
{
	/* C# ObjContainer(string): create a child SpriteObj per CharacterData. */
	const std::vector<SpriteSystem::CharacterData> &charDataList =
		SpriteSystem::SpriteLibrary::GetCharData(spriteName);
	for (const SpriteSystem::CharacterData &chars : charDataList)
	{
		SpriteObj *newObj = new SpriteObj(chars.Child);
		newObj->X((float) chars.ChildX);
		newObj->Y((float) chars.ChildY);
		AddChild(newObj);
	}
	m_numCharData = (int) charDataList.size();
	m_charDataStartIndex = 0;
}

void ObjContainer::ChangeSprite(const std::string &spriteName)
{
	_spriteName = spriteName;
	if (m_charDataStartIndex == -1)
		m_charDataStartIndex = (int) m_objectList.size();

	const std::vector<SpriteSystem::CharacterData> &charDataList =
		SpriteSystem::SpriteLibrary::GetCharData(spriteName);

	int indexCounter = 0;
	for (int i = m_charDataStartIndex; i < (int) charDataList.size(); i++)
	{
		const SpriteSystem::CharacterData &charData = charDataList[indexCounter];
		if (i >= m_numCharData)
		{
			SpriteObj *objToAdd = new SpriteObj(charData.Child);
			objToAdd->X((float) charData.ChildX);
			objToAdd->Y((float) charData.ChildY);
			AddChildAt(i, objToAdd);
			m_numCharData++;
		}
		else
		{
			SpriteObj *spriteObj = static_cast<SpriteObj *>(m_objectList[i]);
			spriteObj->Visible(true);
			spriteObj->ChangeSprite(charData.Child);
			spriteObj->X((float) charData.ChildX);
			spriteObj->Y((float) charData.ChildY);
		}
		indexCounter++;
	}

	if ((int) charDataList.size() < m_numCharData)
	{
		for (int i = (int) charDataList.size(); i < m_numCharData; i++)
			m_objectList[i]->Visible(false);
	}

	StopAnimation();
	CalculateBounds();
}

void ObjContainer::AddChild(GameObj *obj)
{
	m_objectList.push_back(obj);
	obj->Parent(this);
	CalculateBounds();
}

void ObjContainer::AddChildAt(int index, GameObj *obj)
{
	m_objectList.insert(m_objectList.begin() + index, obj);
	obj->Parent(this);
	CalculateBounds();
}

void ObjContainer::RemoveChild(GameObj *obj)
{
	for (size_t i = 0; i < m_objectList.size(); i++)
	{
		if (m_objectList[i] == obj)
		{
			m_objectList.erase(m_objectList.begin() + i);
			break;
		}
	}
	delete obj;
	CalculateBounds();
}

void ObjContainer::RemoveAll()
{
	for (GameObj *obj : m_objectList)
	{
		obj->Parent(nullptr);
		delete obj;
	}
	m_objectList.clear();
	m_bounds = Rectangle();
}

GameObj *ObjContainer::GetChildAt(int index) const
{
	if (index < 0 || index >= (int) m_objectList.size())
		return nullptr;
	return m_objectList[index];
}

void ObjContainer::Draw(Camera2D *camera)
{
	if (!Visible())
		return;
	/* FrameBounds: the room cache fills the letterbox bands, so an
	 * object only inside them (a chandelier at the top of the room)
	 * stays on screen instead of flickering with the camera. */
	if (Bounds().Intersects(camera->FrameBounds()) || ForceDraw())
	{
		for (GameObj *obj : m_objectList)
			obj->Draw(camera);

		/* Container-level lockstep counter (ObjContainer.cs). Children
		 * animate themselves with this container's AnimationDelay. */
		double elapsedTotalSeconds = camera->ElapsedTotalSeconds();
		if (m_animate && m_totalGameTime != elapsedTotalSeconds)
		{
			m_totalGameTime = elapsedTotalSeconds;
			m_timeDelayCounter += (float) elapsedTotalSeconds;
			if (m_timeDelayCounter >= AnimationDelayValue)
			{
				m_timeDelayCounter = 0;
				GoToNextFrame();
			}
		}
	}
}

void ObjContainer::Update(GameTime *gameTime)
{
	(void) gameTime;
	for (GameObj *obj : m_objectList)
		obj->Update(gameTime);
}

void ObjContainer::PlayAnimation(bool loopAnimation)
{
	if (m_charDataStartIndex > -1)
	{
		for (GameObj *obj : m_objectList)
			obj->PlayAnimation(loopAnimation);
		m_loop = loopAnimation;
		m_startAnimationIndex = 1;
		m_endAnimationIndex = TotalFrames();
		m_animate = true;
		m_currentFrameIndex = m_startAnimationIndex;
		m_timeDelayCounter = 0;
	}
}

void ObjContainer::PlayAnimation(int startIndex, int endIndex, bool loopAnimation)
{
	for (GameObj *obj : m_objectList)
		obj->PlayAnimation(startIndex, endIndex, loopAnimation);
	m_loop = loopAnimation;

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

	m_startAnimationIndex = startIndex;
	m_endAnimationIndex = endIndex;
	m_currentFrameIndex = startIndex;
	m_animate = true;
}

void ObjContainer::PlayAnimation(const std::string &startLabel, const std::string &endLabel, bool loopAnimation)
{
	int startIndex = FindLabelIndex(startLabel);
	int endIndex = FindLabelIndex(endLabel);
	if (startIndex != -1 && endIndex != -1)
		PlayAnimation(startIndex, endIndex, loopAnimation);
}

int ObjContainer::FindLabelIndex(const std::string &label) const
{
	for (GameObj *obj : m_objectList)
	{
		int index = obj->FindLabelIndex(label);
		if (index != -1)
			return index;
	}
	return -1;
}

void ObjContainer::StopAnimation()
{
	for (GameObj *obj : m_objectList)
		obj->StopAnimation();
	m_animate = false;
	m_wasAnimating = false;
}

void ObjContainer::PauseAnimation()
{
	m_wasAnimating = m_animate;
	m_animate = false;
	for (GameObj *obj : m_objectList)
		obj->PauseAnimation();
}

void ObjContainer::ResumeAnimation()
{
	for (GameObj *obj : m_objectList)
		obj->ResumeAnimation();
	if (m_wasAnimating)
		m_animate = true;
	m_wasAnimating = false;
}

void ObjContainer::GoToNextFrame()
{
	/* ObjContainer.cs: only tracks the container's frame index. */
	m_currentFrameIndex++;
	if (m_currentFrameIndex > m_endAnimationIndex && m_loop)
		m_currentFrameIndex = m_startAnimationIndex;
	else if (m_currentFrameIndex > m_endAnimationIndex && !m_loop)
		m_animate = false;
}

void ObjContainer::GoToFrame(int index)
{
	for (GameObj *obj : m_objectList)
		obj->GoToFrame(index);
}

int ObjContainer::TotalFrames() const
{
	/* Only returns the largest number of frames in the object list. */
	int totalFrames = 1;
	for (GameObj *obj : m_objectList)
	{
		if (obj->Visible())
		{
			int largestNumberOfFrames = obj->TotalFrames();
			if (largestNumberOfFrames > totalFrames)
				totalFrames = largestNumberOfFrames;
		}
	}
	return totalFrames;
}

void ObjContainer::CalculateBounds()
{
	/* Union of the visible children's bounds (ObjContainer.cs). */
	int leftBound = INT_MAX, rightBound = -INT_MAX;
	int topBound = INT_MAX, bottomBound = -INT_MAX;

	for (GameObj *obj : m_objectList)
	{
		if (obj->Visible() && obj->AddToBounds())
		{
			Rectangle objBounds = obj->Bounds();
			if (objBounds.Left() < leftBound)
				leftBound = objBounds.Left();
			if (objBounds.Right() > rightBound)
				rightBound = objBounds.Right();
			if (objBounds.Top() < topBound)
				topBound = objBounds.Top();
			if (objBounds.Bottom() > bottomBound)
				bottomBound = objBounds.Bottom();
		}
	}

	m_bounds.X = leftBound;
	m_bounds.Y = topBound;
	m_bounds.Width = rightBound - leftBound;
	m_bounds.Height = bottomBound - topBound;

	_width = (int) (m_bounds.Width / ScaleX());
	_height = (int) (m_bounds.Height / ScaleY());
}

} // namespace DS2DEngine
