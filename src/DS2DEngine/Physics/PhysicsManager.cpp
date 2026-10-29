/* Port of DS2DEngine/src/Physics/PhysicsManager.cs. */
#include "PhysicsManager.h"
#include "../../debugflags.h"
#include <cstdio>
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
	std::vector<IPhysicsObj *> &list = obj->IsStatic() ? m_poList : m_dynamicList;
	for (IPhysicsObj *o : list)
		if (o == obj)
			return;
	if (obj->IsStatic())
		m_staticDirty = true;
	else
		m_dynamicList.push_back(obj);
	m_poList.push_back(obj);
}

void PhysicsManager::RemoveObject(IPhysicsObj *obj)
{
	if (obj->IsStatic())
		m_staticDirty = true;
	for (size_t i = 0; i < m_dynamicList.size(); i++)
		if (m_dynamicList[i] == obj)
		{
			m_dynamicList.erase(m_dynamicList.begin() + i);
			break;
		}
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

	/* Only the objects near the active bounds are simulated (see the
	 * header: the whole castle is too much for the pair pass). Rebuilt
	 * here, at the START of the step: objects deleted between frames
	 * (killed projectiles) must not survive in the list. */
	m_active.clear();
	bool hasBounds = m_activeBounds.Width > 0;
	if (m_staticDirty || m_staticBounds.X != m_activeBounds.X || m_staticBounds.Y != m_activeBounds.Y
	    || m_staticBounds.Width != m_activeBounds.Width || m_staticBounds.Height != m_activeBounds.Height)
	{
		m_staticActive.clear();
		for (IPhysicsObj *obj : m_poList)
			if (obj->IsStatic() && (!hasBounds || CollisionMath::Intersects(obj->TerrainBounds(), m_activeBounds)))
				m_staticActive.push_back(obj);
		m_staticBounds = m_activeBounds;
		m_staticDirty = false;
	}
	m_active = m_staticActive;
	for (IPhysicsObj *obj : m_dynamicList)
	{
		if (hasBounds && !CollisionMath::Intersects(obj->TerrainBounds(), m_activeBounds))
			continue;
		m_active.push_back(obj);
	}

	if (rl_trace())
	{
		static int frames = 0;
		if (++frames % 120 == 0) fprintf(stderr, "physics: %zu objects, %zu active\n", m_poList.size(), m_active.size());
	}
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
		}
		/* Unweighted objects (flying enemies) still apply their
		 * acceleration: the knockback (PhysicsManager.cs line 104) */
		weightedObj->UpdatePhysics(gameTime);
		if (!weightedObj->DisableHitboxUpdating())
			weightedObj->UpdateCollisionBoxes();
	}

	/* The rectangle around each object's boxes, a little larger: the
	 * pair pass moves objects (the MTD pushes) after it is built */
	m_activeRects.resize(m_active.size());
	for (size_t i = 0; i < m_active.size(); i++)
	{
		int x0 = INT32_MAX, y0 = INT32_MAX, x1 = INT32_MIN, y1 = INT32_MIN;
		for (CollisionBox *box : m_active[i]->CollisionBoxes())
		{
			if (box->Type() == Consts::NULL_HITBOX) continue;
			Rectangle r = box->AbsRect();
			if (r.X < x0) x0 = r.X;
			if (r.Y < y0) y0 = r.Y;
			if (r.X + r.Width > x1) x1 = r.X + r.Width;
			if (r.Y + r.Height > y1) y1 = r.Y + r.Height;
		}
		m_activeRects[i] = x0 > x1 ? Rectangle(0, 0, 0, 0) : Rectangle(x0 - 16, y0 - 16, x1 - x0 + 32, y1 - y0 + 32);
	}

	/* Pair collision pass (PhysicsManager.cs line 128). */
	for (size_t i = 0; i < m_active.size(); i++)
	{
		if (m_activeRects[i].Width == 0)
			continue;
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
			if (!CollisionMath::Intersects(m_activeRects[i], m_activeRects[j]))
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
						if (rl_hits())
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
