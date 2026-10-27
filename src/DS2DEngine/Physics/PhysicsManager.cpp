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
	for (IPhysicsObj *weightedObj : m_active)
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
		/* Only the objects near the active bounds are simulated (see the
	 * header: the whole castle is too much for the pair pass). */
	m_active.clear();
	bool hasBounds = m_activeBounds.Width > 0;
	for (IPhysicsObj *obj : m_poList)
	{
		if (hasBounds && !CollisionMath::Intersects(obj->TerrainBounds(), m_activeBounds))
			continue;
		m_active.push_back(obj);
	}

	for (size_t i = 0; i < m_active.size(); i++)
	{
		IPhysicsObj *firstObj = m_active[i];
		if (!firstObj->IsCollidable() || !firstObj->Visible())
			continue;
		for (size_t j = i + 1; j < m_active.size(); j++)
		{
			IPhysicsObj *secondObj = m_active[j];

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
					/* Weapon vs body (PhysicsManager.cs line 167): the
					 * weapon owner gets FIRSTBOXHIT, the body owner
					 * SECONDBOXHIT, and vice versa. */
					else if ((firstBox->Type() == Consts::WEAPON_HITBOX && secondBox->Type() == Consts::BODY_HITBOX)
					         || (firstBox->Type() == Consts::BODY_HITBOX && secondBox->Type() == Consts::WEAPON_HITBOX))
					{
						if (CollisionMath::Intersects(firstBox->AbsRect(), secondBox->AbsRect()))
						{
							firstObj->CollisionResponse(firstBox, secondBox, Consts::COLLISIONRESPONSE_TERRAIN);
							secondObj->CollisionResponse(secondBox, firstBox, Consts::COLLISIONRESPONSE_TERRAIN);
						}
					}
					/* Weapon vs body (PhysicsManager.cs line 167): the
					 * weapon owner gets FIRSTBOXHIT, the body owner
					 * SECONDBOXHIT, and vice versa. */
					else if ((firstBox->Type() == Consts::WEAPON_HITBOX && secondBox->Type() == Consts::BODY_HITBOX)
					         || (firstBox->Type() == Consts::BODY_HITBOX && secondBox->Type() == Consts::WEAPON_HITBOX))
					{
						if (getenv("RL_DEBUG_HITS") != nullptr)
							fprintf(stderr, "wxb: t1=%d t2=%d f=[%d,%d %dx%d] s=[%d,%d %dx%d] int=%d\n",
							        firstObj->CollisionTypeTag(), secondObj->CollisionTypeTag(),
							        firstBox->AbsRect().X, firstBox->AbsRect().Y, firstBox->AbsRect().Width, firstBox->AbsRect().Height,
							        secondBox->AbsRect().X, secondBox->AbsRect().Y, secondBox->AbsRect().Width, secondBox->AbsRect().Height,
							        (int) CollisionMath::Intersects(firstBox->AbsRect(), secondBox->AbsRect()));
						if (CollisionMath::Intersects(firstBox->AbsRect(), secondBox->AbsRect()))
						{
							/* PhysicsManager.cs line 170: the object whose
							 * box is the BODY receives FIRSTBOXHIT, the one
							 * whose box is the WEAPON receives SECONDBOXHIT
							 * (EnemyObj handles FIRSTBOXHIT = "player has
							 * hit enemy", PlayerObj checks the box types). */
							if (firstBox->Type() == Consts::WEAPON_HITBOX)
							{
								firstObj->CollisionResponse(firstBox, secondBox, Consts::COLLISIONRESPONSE_SECONDBOXHIT);
								secondObj->CollisionResponse(secondBox, firstBox, Consts::COLLISIONRESPONSE_FIRSTBOXHIT);
							}
							else
							{
								firstObj->CollisionResponse(firstBox, secondBox, Consts::COLLISIONRESPONSE_FIRSTBOXHIT);
								secondObj->CollisionResponse(secondBox, firstBox, Consts::COLLISIONRESPONSE_SECONDBOXHIT);
							}
						}
					}
				}
			}
		}
	}
}

} // namespace DS2DEngine
