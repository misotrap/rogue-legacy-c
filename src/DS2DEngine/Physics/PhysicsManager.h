/* Port of DS2DEngine/src/Physics/PhysicsManager.cs: gravity, terminal
 * velocity and the object-pair collision pass (terrain vs terrain,
 * weapon vs body later with combat).
 */
#ifndef DS2D_PHYSICS_MANAGER_H
#define DS2D_PHYSICS_MANAGER_H

#include "IPhysicsObj.h"
#include "../Types.h"
#include <vector>

namespace DS2DEngine
{

class PhysicsManager
{
public:
	PhysicsManager();

	void AddObject(IPhysicsObj *obj);
	void RemoveObject(IPhysicsObj *obj);
	void Update(GameTime *gameTime);

	int NumObjects() const { return (int) m_poList.size(); }
	void SetGravity(float xGrav, float yGrav);

	/* A whole castle is too many objects for the O(n*n) pair pass (the
	 * Miyoo has no chance); the level restricts the simulation to the
	 * objects near the current room. An empty rectangle disables the
	 * filter (all objects active). */
	void SetActiveBounds(const Rectangle &bounds) { m_activeBounds = bounds; }
	const Rectangle &ActiveBounds() const { return m_activeBounds; }
	Vector2 GetGravity() const { return m_gravity; }
	int TerminalVelocity = 1000;

private:
	std::vector<IPhysicsObj *> m_poList;
	std::vector<IPhysicsObj *> m_active;
	Rectangle m_activeBounds;
	Vector2 m_gravity;
};

} // namespace DS2DEngine

#endif
