/* Port of RogueCastle/src/Screens/ProceduralLevelScreen.cs (subset): owns
 * the generated castle, the player, the physics manager and the camera.
 * Enemies, HUD, projectiles and the special rooms come with their
 * milestones.
 */
#ifndef ROGUE_CASTLE_PROCEDURAL_LEVEL_SCREEN_H
#define ROGUE_CASTLE_PROCEDURAL_LEVEL_SCREEN_H

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/Physics/PhysicsManager.h"
#include "../GameObjects/Player/PlayerObj.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../Levels/LevelBuilder2.h"
#include "../Levels/LevelParser.h"
#include "../Rendering/RoomRenderer.h"
#include <vector>

namespace RogueCastle
{

class ProceduralLevelScreen
{
public:
	/* Loads assets/rooms.txt and builds the castle. seed 0 = time based. */
	ProceduralLevelScreen(const char *assets, uint32_t seed);

	void Update(GameTime *gameTime);
	void Draw(DS2DEngine::Camera2D *camera);

	/* The ProjectileManager subset: bones and later the spells. */
	void FireProjectile(const char *spriteName, Vector2 position,
	                    Vector2 heading, float speed, int damage,
	                    Vector2 scale, bool weighted, class EnemyObj *source,
	                    float lifeSpan = 0);

	PlayerObj *Player() const { return m_player; }
	RoomObj *CurrentRoom() const { return m_currentRoom; }
	const std::vector<RoomObj *> &RoomList() const { return m_roomList; }
	DS2DEngine::Camera2D *Camera() { return &m_camera; }
	bool IsValid() const { return m_valid; }

private:
	void UpdateCamera();               /* ProceduralLevelScreen.cs line 1621 */
	void CheckForRoomTransition();     /* ProceduralLevelScreen.cs line 1189 */

	std::vector<class ProjectileObj *> m_projectiles;   /* owned */
	Levels::Castle m_castle;
	RoomRenderer m_roomRenderer;   /* the original look behind the room objects */
	std::vector<RoomObj *> m_roomList;
	std::vector<DS2DEngine::IPhysicsObj *> m_allTerrain;
	RoomObj *m_currentRoom = nullptr;
	PlayerObj *m_player = nullptr;
	DS2DEngine::PhysicsManager m_physicsManager;
	DS2DEngine::Camera2D m_camera;     /* C#: ScreenManager.Camera */
	bool m_valid = false;
};

} // namespace RogueCastle

#endif
