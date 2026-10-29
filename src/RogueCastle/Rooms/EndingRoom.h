/* Port of RogueCastle/src/Game Objects/RoomObjs/EndingRoomObj.cs: after
 * the last boss dies, the camera walks through the gallery of every
 * enemy of the game, each in its frame with its name and how many were
 * slain, before the credits. A separate screen like the credits: the
 * castle level is gone, this builds its own room from the "Ending"
 * template of rooms.txt (LevelBuilder2.CreateEndingRoom moves it to
 * (0,0), the port's templates already carry local coordinates).
 *
 * Not ported: the DropShadow of the frames (no shadow blit), the
 * m_blobBoss follow-up tweaks (its colour fix is applied), the
 * Eyeball pupil swap (ChangeToBossPupil) and the LastBoss
 * ForceSecondForm flag (its idle sprite is used directly).
 */
#ifndef ROGUE_CASTLE_ENDING_ROOM_H
#define ROGUE_CASTLE_ENDING_ROOM_H

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include "../Save/SaveGame.h"

#include <cstdint>
#include <string>
#include <vector>

namespace RogueCastle
{

class RoomObj;
class EnemyObj;

class EndingRoom
{
public:
	~EndingRoom();
	bool LoadContent(const char *assetsDir);
	/* EndingRoomObj.cs LoadContent + OnEnter: builds the gallery from
	 * the Ending template and starts at the first waypoint. */
	void Open(const char *assetsDir, Save::GameState &save);
	/* One step; returns true when the credits follow (the last waypoint
	 * or the continue skip). */
	bool Update(uint32_t pressed, double dt);
	void Draw(uint32_t *frame);
	bool IsOpen() const { return m_room != nullptr; }

private:
	/* One framed enemy of the gallery. */
	struct Exhibit
	{
		EnemyObj *enemy = nullptr;   /* owned by m_room */
		DS2DEngine::SpriteObj *frame = nullptr;
		DS2DEngine::SpriteObj *plaque = nullptr;
		std::string name, slain;
		float x = 0, nameY = 0, slainY = 0;
	};

	RoomObj *m_room = nullptr;   /* owns the enemies and decorations */
	std::vector<Exhibit> m_exhibits;
	std::vector<Vector2> m_waypoints;   /* the EnemyOrbObj waypoints, by Name index */
	DS2DEngine::Camera2D m_camera;
	Text::Font m_font;
	DS2DEngine::SpriteObj *m_bg = nullptr;   /* LineageScreenBG_Sprite, repeated */

	size_t m_waypointIndex = 1;
	double m_waypointTime = 0;   /* since the tween to this waypoint began */
	/* The continue hint (Update: any button shows it, the next press
	 * leaves for the credits). */
	bool m_continueShown = false;
	double m_continueTimer = 0;
	float m_continueOpacity = 0;
};

} // namespace RogueCastle

#endif
