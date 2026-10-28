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
	/* No terrain box in any part (the flying portraits): the overall
	 * bounds, so the object still sits where it is (active area culling) */
	if (leftBound == INT_MAX || topBound == INT_MAX)
		bounds = Bounds();
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

void PhysicsObjContainer::UpdateTerrainBox()
{
	int leftBound = INT_MAX, rightBound = -INT_MAX;
	int topBound = INT_MAX, bottomBound = -INT_MAX;
	for (CollisionBox *box : m_collisionBoxes)
	{
		if (box->Type() != Consts::TERRAIN_HITBOX)
			continue;
		Rectangle r = box->AbsRect();
		if (r.Left() < leftBound)
			leftBound = r.Left();
		if (r.Right() > rightBound)
			rightBound = r.Right();
		if (r.Top() < topBound)
			topBound = r.Top();
		if (r.Bottom() > bottomBound)
			bottomBound = r.Bottom();
	}
	if (leftBound == INT_MAX)
		return;
	m_terrainBounds = Rectangle(leftBound, topBound, rightBound - leftBound, bottomBound - topBound);
}

void PhysicsObjContainer::AddCollisionBox(int xPos, int yPos, int width, int height, int hitboxType)
{
	CollisionBox *box = new CollisionBox(xPos, yPos, width, height, hitboxType, this);
	box->PhysicsParent(this);
	box->WasAdded = true;
	m_collisionBoxes.push_back(box);
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
