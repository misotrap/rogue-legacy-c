/* Port of DS2DEngine/src/Physics/PhysicsManager.cs (subset). */
#include "PhysicsManager.h"

namespace DS2DEngine
{

PhysicsManager::PhysicsManager()
	: m_gravity(0, 0)
{
	TerminalVelocity = 1000;
}

void PhysicsManager::AddObject(PhysicsObjContainer *obj)
{
	for (PhysicsObjContainer *o : m_poList)
		if (o == obj)
			return;
	m_poList.push_back(obj);
}

void PhysicsManager::RemoveObject(PhysicsObjContainer *obj)
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

	for (PhysicsObjContainer *weightedObj : m_poList)
	{
		if (!weightedObj->Visible())
			continue;
		if (weightedObj->DisableAllWeight)
			continue;
		if (weightedObj->IsWeighted)
		{
			Vector2 gravity(m_gravity.X * elapsedTime, m_gravity.Y * elapsedTime);
			if (weightedObj->AccelerationXEnabled && !weightedObj->DisableGravity)
				weightedObj->AccelerationX(weightedObj->AccelerationX() + gravity.X);
			if (weightedObj->AccelerationYEnabled && !weightedObj->DisableGravity)
				weightedObj->AccelerationY(weightedObj->AccelerationY() + gravity.Y);
			if (weightedObj->AccelerationY() > (float) TerminalVelocity)
				weightedObj->AccelerationY((float) TerminalVelocity);

			weightedObj->UpdatePhysics(gameTime);
		}
		if (!weightedObj->DisableHitboxUpdating)
			weightedObj->UpdateCollisionBoxes();
	}
}

} // namespace DS2DEngine
