/* Port of DS2DEngine/src/GameObjs/PhysicsObjContainer.cs. */
#include "PhysicsObjContainer.h"
#include "../Consts.h"
#include <climits>

namespace DS2DEngine
{

PhysicsObjContainer::PhysicsObjContainer()
{
	IsWeighted = true;
	IsCollidable = true;
	CollidesLeft = true;
	CollidesRight = true;
	CollidesTop = true;
	CollidesBottom = true;
	AccelerationXEnabled = true;
	AccelerationYEnabled = true;
	m_charDataStartIndex = -1;
}

PhysicsObjContainer::PhysicsObjContainer(const std::string &spriteName)
{
	IsWeighted = true;
	IsCollidable = true;
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
	if (AccelerationXEnabled)
		X(X() + m_acceleration.X * elapsedSeconds);
	if (AccelerationYEnabled)
		Y(Y() + m_acceleration.Y * elapsedSeconds);
}

void PhysicsObjContainer::CollisionResponseTerrain(CollisionBox *thisBox, CollisionBox *otherBox)
{
	/* Ported in milestone 1 with the room collision code. */
	(void) thisBox;
	(void) otherBox;
}

} // namespace DS2DEngine
