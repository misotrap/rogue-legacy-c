/* Port of RogueCastle/src/Screens/ProceduralLevelScreen.cs (subset): owns
 * the generated castle, the player, the physics manager and the camera.
 * Enemies, HUD, projectiles and the special rooms come with their
 * milestones.
 */
#ifndef ROGUE_CASTLE_PROCEDURAL_LEVEL_SCREEN_H
#define ROGUE_CASTLE_PROCEDURAL_LEVEL_SCREEN_H

#include "../../DS2DEngine/Camera2D.h"
#include "../../Text/Font.h"
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
	/* Loads assets/rooms.txt and builds the castle. seed 0 = time based.
	 * chestFlags (optional, owned by the caller, e.g. save.chestsOpened):
	 * one flag per chest, in room order; opened chests stay open. */
	ProceduralLevelScreen(const char *assets, uint32_t seed,
	                      std::vector<uint8_t> *chestFlags = nullptr, int timesCastleBeaten = 0);
	~ProceduralLevelScreen();   /* frees the fonts (2 MB of Junicode) */

	void Update(GameTime *gameTime);
	void Draw(DS2DEngine::Camera2D *camera);

	/* The ProjectileManager subset: bones and later the spells. */
	void AddProjectile(class ProjectileObj *proj);   /* takes ownership */
	/* ProjectileManager.cs line 258 (FireProjectile) for the player's
	 * spells: position from the source anchor and flip, heading from the
	 * angle degrees, the player tag. */
	class ProjectileObj *FireSpellProjectile(const struct ProjectileData &data,
	                                        uint8_t spell);
	/* The nuke spell (ProjectileObj.cs line 578: the whole room). */
	void DamageAllEnemies(int damage);
	/* The time stop spell: the enemies freeze for TIMESTOP_XVal seconds. */
	void CastTimeStop();
	void StopTimeStop();
	bool IsTimeStopped() const { return m_timeStopped; }
	/* The ItemDropManager subset (ItemDropManager.cs): the loot. */
	void DropItem(Vector2 position, int dropType, float amount);
	void DropItemWide(Vector2 position, int dropType, float amount);
	/* TextManager.DisplayNumberStringText: the floating pickup texts. */
	void AddPopup(const std::string &text, float x, float y, uint32_t colour);
	/* The fairy chest objective plate (ProceduralLevelScreen.cs lines
	 * 2870-2935): slides in from the right when the room has a locked
	 * fairy chest, shows the objective, the failure and the success. */
	void DisplayObjective(const char *titleID, const char *description,
	                      const char *progress, bool tween);
	void ResetObjectivePlate(bool tween);
	void UpdateObjectiveProgress(const std::string &progress);
	void ObjectiveFailed();
	void ObjectiveComplete();
	/* Dev/test (RL_DEBUG_CHEST): "12p" opens chest 12, forcing a print. */
	void DebugOpenChest(const char *arg);
	/* The shots of this enemy vanish (EnemyObj_Energon.DestroyAllProjectiles) */
	void KillProjectilesFrom(class EnemyObj *source);
	class ProjectileObj *FireProjectile(const char *spriteName, Vector2 position,
	                    Vector2 heading, float speed, int damage,
	                    Vector2 scale, bool weighted, class EnemyObj *source,
	                    float lifeSpan = 0, bool collidesWithTerrain = false);

	PlayerObj *Player() const { return m_player; }
	const Levels::Castle &Castle() const { return m_castle; }
	RoomObj *CurrentRoom() const { return m_currentRoom; }
	const std::vector<RoomObj *> &RoomList() const { return m_roomList; }
	DS2DEngine::Camera2D *Camera() { return &m_camera; }
	bool IsValid() const { return m_valid; }
	void UpdateCamera();               /* ProceduralLevelScreen.cs line 1621 */
	void SpawnDementiaEnemy();         /* ProceduralLevelScreen.cs line 2382 */
	/* AddEnemyToCurrentRoom (line 2414): a temporary enemy of the given type
	 * in the hero's room, wired to the hero, the terrain and the physics */
	class EnemyObj *AddEnemyToCurrentRoom(uint8_t type, Vector2 position, const char *difficulty = "BASIC");

private:
	void CheckForRoomTransition();     /* ProceduralLevelScreen.cs line 1189 */
	/* ProceduralLevelScreen.cs line 742: the chest colour roll and level. */
	void InitializeChests();

	std::vector<class ProjectileObj *> m_projectiles;   /* owned */
	std::vector<class ItemDropObj *> m_itemDrops;       /* owned */
	/* DisplayNumberStringText: the floating pickup texts. */
	struct Popup { std::string text; float x, y, timer; uint32_t colour; };
	std::vector<Popup> m_popups;
	Text::Font m_goldFont;   /* the gold numbers, like the HUD */
	Text::Font m_objFont;   /* the objective plate (Junicode, like the C#) */
	DS2DEngine::SpriteObj *m_objPlate = nullptr;   /* DialogBox_Sprite */
	DS2DEngine::Camera2D m_objCamera;   /* zoom 1: world = screen */
	std::string m_objTitle, m_objDescription, m_objProgress;
	float m_objAlphaTitle = 1;   /* the 0.3 dim of ObjectiveFailed/Complete */
	float m_objX = 1470;         /* the plate's slide (virtual X) */
	bool m_objShown = false;
	double m_objTime = 0;        /* the plate's clock */
	double m_objSlideStart = 0;
	float m_objSlideFrom = 0;
	float m_objSlideTo = 0;
	Levels::Castle m_castle;
	RoomRenderer m_roomRenderer;   /* the original look behind the room objects */
	std::vector<RoomObj *> m_roomList;
	std::vector<class ChestObj *> m_chestList;   /* the rooms own them */
	std::vector<uint8_t> *m_chestFlags = nullptr;   /* save.chestsOpened */
	/* Every floor and wall of the castle; m_allTerrain holds those around
	 * the current room (the ground probes of the hero and the enemies),
	 * refilled on each room change */
	std::vector<DS2DEngine::IPhysicsObj *> m_castleTerrain;
	std::vector<DS2DEngine::IPhysicsObj *> m_allTerrain;
	void RefreshLocalTerrain();
	/* RL_NO_ROOM_CACHE: draw the rooms live (to compare) */
	bool m_noRoomCache = getenv("RL_NO_ROOM_CACHE") != nullptr;
	std::vector<Slope> m_allSlopes;
	std::vector<Hazard> m_allHazards;
	RoomObj *m_currentRoom = nullptr;
	bool m_timeStopped = false;  /* the time stop spell (a flag: it lasts
	                              while the hero's mana pays the drain) */
	PlayerObj *m_player = nullptr;
	DS2DEngine::PhysicsManager m_physicsManager;
	DS2DEngine::Camera2D m_camera;     /* C#: ScreenManager.Camera */
	bool m_valid = false;
};

} // namespace RogueCastle

#endif
