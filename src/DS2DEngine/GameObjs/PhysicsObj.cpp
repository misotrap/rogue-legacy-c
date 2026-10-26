/* Port of DS2DEngine/src/GameObjs/PhysicsObj.cs. */
#include "PhysicsObj.h"
#include "ObjContainer.h"
#include "PhysicsObjContainer.h"
#include "../Consts.h"
#include <climits>

namespace DS2DEngine
{

PhysicsObj::PhysicsObj(const std::string &spriteName)
	: SpriteObj(spriteName)
{
	m_terrainBounds = Rectangle();
}

PhysicsObj::~PhysicsObj()
{
	ClearCollisionBoxes();
}

void PhysicsObj::UpdatePhysics(GameTime *gameTime)
{
	float elapsedSeconds = (float) gameTime->ElapsedGameTime;
	if (AccelerationXEnabled)
		X(X() + m_acceleration.X * elapsedSeconds);
	if (AccelerationYEnabled)
		Y(Y() + m_acceleration.Y * elapsedSeconds);
}

void PhysicsObj::UpdateCollisionBoxes()
{
	/* PhysicsObj.cs: rebuild the collision boxes from the current frame's
	 * hitboxes, scaled by the parent container's scale. The boxes are
	 * relative to this object's origin (anchor point). */
	if (m_frameDataList == nullptr || _frameIndex >= (int) m_frameDataList->size())
		return;
	const std::vector<SpriteSystem::Hitbox> &hbList = (*m_frameDataList)[_frameIndex].hitboxList;

	Rectangle bounds = Rectangle();
	int leftBound = INT_MAX, rightBound = -INT_MAX;
	int topBound = INT_MAX, bottomBound = -INT_MAX;

	float parentScaleX = Parent() != nullptr ? Parent()->ScaleX() : ScaleX();
	float parentScaleY = Parent() != nullptr ? Parent()->ScaleY() : ScaleY();

	for (size_t i = 0; i < hbList.size(); i++)
	{
		if (m_collisionBoxes.size() <= i)
		{
			CollisionBox *box = new CollisionBox(this);
			m_collisionBoxes.push_back(box);
			/* Passes by reference in C#: the parent container shares the
			 * same boxes (PhysicsObjContainer keeps its own list). */
			PhysicsObjContainer *container = Parent() != nullptr ? Parent()->AsPhysicsObjContainer() : nullptr;
			if (container != nullptr)
				container->AddSharedCollisionBox(box);
		}
		CollisionBox *box = m_collisionBoxes[i];
		box->X((int) (hbList[i].X * parentScaleX * ScaleX()));
		box->Y((int) (hbList[i].Y * parentScaleY * ScaleY()));
		box->Width((int) (hbList[i].Width * parentScaleX * ScaleX()));
		box->Height((int) (hbList[i].Height * parentScaleY * ScaleY()));
		box->InternalRotation(hbList[i].Rotation);
		box->Type(hbList[i].Type);

		if (box->Type() == Consts::TERRAIN_HITBOX)
		{
			Rectangle absRect = box->AbsRect();
			if (absRect.Left() < leftBound)
				leftBound = absRect.Left();
			if (absRect.Right() > rightBound)
				rightBound = absRect.Right();
			if (absRect.Top() < topBound)
				topBound = absRect.Top();
			if (absRect.Bottom() > bottomBound)
				bottomBound = absRect.Bottom();
		}
	}

	bounds.X = leftBound;
	bounds.Y = topBound;
	bounds.Width = rightBound - leftBound;
	bounds.Height = bottomBound - topBound;
	m_terrainBounds = bounds;

	/* Clear out any collision boxes not being used. */
	for (size_t k = hbList.size(); k < m_collisionBoxes.size(); k++)
	{
		if (!m_collisionBoxes[k]->WasAdded)
		{
			m_collisionBoxes[k]->X(0);
			m_collisionBoxes[k]->Y(0);
			m_collisionBoxes[k]->Width(1);
			m_collisionBoxes[k]->Height(1);
			m_collisionBoxes[k]->InternalRotation(0);
			m_collisionBoxes[k]->Type(Consts::NULL_HITBOX);
		}
	}
}

void PhysicsObj::AddCollisionBox(int xPos, int yPos, int width, int height, int hitboxType)
{
	CollisionBox *box = new CollisionBox((int) (xPos - AnchorX()), (int) (yPos - AnchorY()),
	                                     width, height, hitboxType, this);
	box->WasAdded = true;
	m_collisionBoxes.push_back(box);
}

void PhysicsObj::ClearCollisionBoxes()
{
	for (CollisionBox *box : m_collisionBoxes)
		delete box;
	m_collisionBoxes.clear();
}

} // namespace DS2DEngine
