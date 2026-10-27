/* Port of RogueCastle/src/Game Objects/BreakableObj.cs. */
#include "BreakableObj.h"
#include "../../DS2DEngine/Consts.h"

namespace RogueCastle
{

using namespace DS2DEngine;

BreakableObj::BreakableObj(const std::string &spriteName)
	: PhysicsObjContainer(spriteName)
{
	/* BreakableObj.cs constructor. */
	CollisionTypeTag(5);       /* GameTypes.CollisionType_WALL_FOR_PLAYER */
	SameTypesCollideFlag = true;
	CollidesLeftFlag = false;
	CollidesRightFlag = false;
	CollidesBottomFlag = false;

	/* Only the first part shows while intact; Break() shows the rest. */
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->Visible(i == 0);
}

void BreakableObj::CollisionResponse(CollisionBox *thisBox, CollisionBox *otherBox, int collisionResponseType)
{
	/* BreakableObj.cs line 36. */
	if (otherBox->PhysicsParent() != nullptr && otherBox->Type() == Consts::WEAPON_HITBOX
	    && !HitBySpellsOnly)
	{
		if (!Broken && otherBox->PhysicsParent()->CollisionTypeTag() == 2 /* player */)
			Break();
	}

	/* Hack to make only terrain or other breakables collide with this:
	 * pushed only by something above (or rotated, not supported). */
	if (collisionResponseType == Consts::COLLISIONRESPONSE_TERRAIN
	    && otherBox->AbsRect().Y >= thisBox->AbsRect().Y)
		PhysicsObjContainer::CollisionResponse(thisBox, otherBox, collisionResponseType);
}

void BreakableObj::Break()
{

	/* BreakableObj.cs line 61. TODO C++: item drops (ItemDropManager). */
	for (int i = 0; i < NumChildren(); i++)
		GetChildAt(i)->Visible(true);

	GoToFrame(2);
	Broken = true;
	m_wasWeighted = IsWeightedFlag;
	IsWeightedFlag = false;
	IsCollidableFlag = false;
}

} // namespace RogueCastle
