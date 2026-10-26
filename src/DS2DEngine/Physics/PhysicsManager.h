/* Port of DS2DEngine/src/Physics/PhysicsManager.cs (subset): applies
 * gravity and terminal velocity to every weighted object and steps their
 * physics. Object-vs-object collision comes with the rooms (milestone 1).
 */
#ifndef DS2D_PHYSICS_MANAGER_H
#define DS2D_PHYSICS_MANAGER_H

#include "../GameObjs/PhysicsObjContainer.h"
#include "../Types.h"
#include <vector>

namespace DS2DEngine
{

class PhysicsManager
{
public:
	PhysicsManager();

	void AddObject(PhysicsObjContainer *obj);
	void RemoveObject(PhysicsObjContainer *obj);
	void Update(GameTime *gameTime);

	void SetGravity(float xGrav, float yGrav);
	Vector2 GetGravity() const { return m_gravity; }
	int TerminalVelocity = 1000;

private:
	std::vector<PhysicsObjContainer *> m_poList;
	Vector2 m_gravity;
};

} // namespace DS2DEngine

#endif
