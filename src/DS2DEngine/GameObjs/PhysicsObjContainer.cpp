/* Port of DS2DEngine/src/GameObjs/PhysicsObjContainer.cs. */
#include "PhysicsObjContainer.h"
#include "../CollisionMath.h"
#include "../Consts.h"
#include <climits>

namespace DS2DEngine
{

PhysicsObjContainer::PhysicsObjContainer()
{
	IsWeightedFlag = true;
	IsCollidableFlag = true;
	m_charDataStartIndex = -1;
}

PhysicsObjContainer::PhysicsObjContainer(const std::string &spriteName)
{
	IsWeightedFlag = true;
	IsCollidableFlag = true;
	m_charDataStartIndex = -1;
	_spriteName = spriteName; /* Do not remove this line. */
	BuildFromCharData(spriteName);
}

void PhysicsObjContainer::BuildFromCharData(const std::string &spriteName)
{
	/* This container needs PhysicsObjs as children (PhysicsObjContainer.cs). */
	const std::vector<SpriteSystem::CharacterData> &charDataList =
		SpriteSystem::SpriteLibrary::GetCharData(spriteName);
	for (const SpriteSystem::CharacterData &chars : charDataList)
	{
		PhysicsObj *newObj = new PhysicsObj(chars.Child);
		newObj->X((float) chars.ChildX);
		newObj->Y((float) chars.ChildY);
		AddChild(newObj);
	}
	m_numCharData = (int) charDataList.size();
	m_charDataStartIndex = 0;
}

void PhysicsObjContainer::UpdateCollisionBoxes()
{
	/* Union of the children's terrain bounds (PhysicsObjContainer.cs). */
	Rectangle bounds;
	int leftBound = INT_MAX, rightBound = -INT_MAX;
	int topBound = INT_MAX, bottomBound = -INT_MAX;

	for (GameObj *obj : m_objectList)
	{
		PhysicsObj *physObj = obj->AsPhysicsObj();
		if (physObj != nullptr)
		{
			physObj->UpdateCollisionBoxes();
			Rectangle tb = physObj->TerrainBounds();
			if (tb.Left() < leftBound)
				leftBound = tb.Left();
			if (tb.Right() > rightBound)
				rightBound = tb.Right();
			if (tb.Top() < topBound)
				topBound = tb.Top();
			if (tb.Bottom() > bottomBound)
				bottomBound = tb.Bottom();
		}
	}

	bounds.X = leftBound;
	bounds.Y = topBound;
	bounds.Width = rightBound - leftBound;
	bounds.Height = bottomBound - topBound;
	m_terrainBounds = bounds;
}

void PhysicsObjContainer::UpdatePhysics(GameTime *gameTime)
{
	float elapsedSeconds = (float) gameTime->ElapsedGameTime;
	if (AccelerationXFlag)
		X(X() + m_acceleration.X * elapsedSeconds);
	if (AccelerationYFlag)
		Y(Y() + m_acceleration.Y * elapsedSeconds);
}

void PhysicsObjContainer::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType)
{
	/* PhysicsObjContainer.cs line 179 (CollisionResponse). */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN && IsWeightedFlag)
	{
		Vector2 mtdPos = CollisionMath::CalculateMTD(thisBox->AbsRect(), otherBox->AbsRect());
		IPhysicsObj *otherPhysicsObj = otherBox->PhysicsParent();
		if (otherPhysicsObj == nullptr)
			return;
		int collisionMarginOfError = 10;
		if (mtdPos.Y < 0)   // There is an object below this object.
		{
			/* This used to be AccelerationY > 0; changed for player dashing. */
			if (AccelerationY() >= 0 && otherPhysicsObj->CollidesTop() && IsWeightedFlag)
			{
				/* This code is for dropping through blocks: only stop the
				 * fall when the object really overlaps the platform, not
				 * when just brushing its edge. */
				Rectangle myTB = TerrainBounds();
				Rectangle otherTB = otherPhysicsObj->TerrainBounds();
				if (otherBox->AbsRotation() != 0.0f ||
				    (otherTB.Left() <= myTB.Right() && otherTB.Right() >= myTB.Right() &&
				     myTB.Right() - otherTB.Left() > collisionMarginOfError) ||
				    (myTB.Left() <= otherTB.Right() && myTB.Right() >= otherTB.Right() &&
				     otherTB.Right() - myTB.Left() > collisionMarginOfError))
				{
					AccelerationY(0);
					Y(Y() + mtdPos.Y);
				}
			}
		}
		else if (mtdPos.Y > 0)   // There is an object above this object.
		{
			if (AccelerationY() < 0 && otherPhysicsObj->CollidesBottom() && IsWeightedFlag)
			{
				AccelerationY(0);
				Y(Y() + mtdPos.Y);
			}
		}

		if (mtdPos.X != 0)
		{
			if ((otherPhysicsObj->CollidesLeft() && mtdPos.X > 0) ||
			    (otherPhysicsObj->CollidesRight() && mtdPos.X < 0))
			{
				AccelerationX(0);
				X(X() + mtdPos.X);
			}
		}
	}
}

} // namespace DS2DEngine
