/* Port of DS2DEngine/src/Physics/PhysicsManager.cs. */
#include "PhysicsManager.h"
#include "../CollisionMath.h"
#include "../Consts.h"

namespace DS2DEngine
{

PhysicsManager::PhysicsManager()
	: m_gravity(0, 0)
{
	TerminalVelocity = 1000;
}

void PhysicsManager::AddObject(IPhysicsObj *obj)
{
	for (IPhysicsObj *o : m_poList)
		if (o == obj)
			return;
	m_poList.push_back(obj);
}

void PhysicsManager::RemoveObject(IPhysicsObj *obj)
{
	for (size_t i = 0; i < m_poList.size(); i++)
	{
		if (m_poList[i] == obj)
		{
			m_poList.erase(m_poList.begin() + i);
			return;
		}
	}
}

void PhysicsManager::SetGravity(float xGrav, float yGrav)
{
	m_gravity = Vector2(xGrav, yGrav);
}

void PhysicsManager::Update(GameTime *gameTime)
{
	float elapsedTime = (float) gameTime->ElapsedGameTime;

	/* Gravity + physics step (PhysicsManager.cs line 76). */
	for (IPhysicsObj *weightedObj : m_poList)
	{
		if (!weightedObj->Visible())
			continue;
		if (weightedObj->DisableAllWeight())
			continue;
		if (weightedObj->IsWeighted())
		{
			Vector2 gravity(m_gravity.X * elapsedTime, m_gravity.Y * elapsedTime);
			if (weightedObj->AccelerationXEnabled() && !weightedObj->DisableGravity())
				weightedObj->AccelerationX(weightedObj->AccelerationX() + gravity.X);
			if (weightedObj->AccelerationYEnabled() && !weightedObj->DisableGravity())
				weightedObj->AccelerationY(weightedObj->AccelerationY() + gravity.Y);
			if (weightedObj->AccelerationY() > (float) TerminalVelocity)
				weightedObj->AccelerationY((float) TerminalVelocity);

			weightedObj->UpdatePhysics(gameTime);
		}
		if (!weightedObj->DisableHitboxUpdating())
			weightedObj->UpdateCollisionBoxes();
	}

	/* Pair collision pass (PhysicsManager.cs line 128): terrain-vs-terrain
	 * pairs push the weighted object out; weapon-vs-body will come with the
	 * combat milestone. */
	for (size_t i = 0; i < m_poList.size(); i++)
	{
		IPhysicsObj *firstObj = m_poList[i];
		if (!firstObj->IsCollidable() || !firstObj->Visible())
			continue;
		for (size_t j = i + 1; j < m_poList.size(); j++)
		{
			IPhysicsObj *secondObj = m_poList[j];

			/* Same-type objects (walls with walls, enemies with enemies)
			 * do not collide unless they opt in. */
			if (firstObj->CollisionTypeTag() == secondObj->CollisionTypeTag() &&
			    !firstObj->SameTypesCollide() && !secondObj->SameTypesCollide())
				continue;
			if (!secondObj->IsCollidable() || !secondObj->Visible())
				continue;

			for (CollisionBox *firstBox : firstObj->CollisionBoxes())
			{
				if (firstBox->Type() == Consts::NULL_HITBOX)
					continue;
				for (CollisionBox *secondBox : secondObj->CollisionBoxes())
				{
					if (secondBox->Type() == Consts::NULL_HITBOX)
						continue;
					if (firstBox->Type() == Consts::TERRAIN_HITBOX && secondBox->Type() == Consts::TERRAIN_HITBOX)
					{
						if (CollisionMath::Intersects(firstBox->AbsRect(), secondBox->AbsRect()))
						{
							firstObj->CollisionResponse(firstBox, secondBox, Consts::COLLISIONRESPONSE_TERRAIN);
							secondObj->CollisionResponse(secondBox, firstBox, Consts::COLLISIONRESPONSE_TERRAIN);
						}
					}
					/* TODO C++: COLLISIONRESPONSE_FIRSTBOXHIT/SECONDBOXHIT
					 * (weapon vs body) with the combat milestone. */
				}
			}
		}
	}
}

} // namespace DS2DEngine
