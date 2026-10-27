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

	void SetGravity(float xGrav, float yGrav);
	Vector2 GetGravity() const { return m_gravity; }
	int TerminalVelocity = 1000;

private:
	std::vector<IPhysicsObj *> m_poList;
	Vector2 m_gravity;
};

} // namespace DS2DEngine

#endif
