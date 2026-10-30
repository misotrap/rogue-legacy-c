// The castle map (MapScreen.cs + MapObj.cs), opened with Select in the
// castle. Only the visited rooms are shown, each as its MapRoom{w}x{h}
// sprite tinted with its area colour (white castle, green garden, dark
// blue tower, red dungeon), with its doors, the boss icon on the boss
// entrances and the blinking player icon. One room unit (1320x720) is
// 60x32 map pixels (m_spriteScale 22 x 22.5), drawn 1:1 on the screen.
// The game shows through at 30%. D-pad: move the map; A: recentre;
// Select or B: close.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"
#include "../Levels/RoomData.h"

#include <cstdint>
#include <vector>

namespace RogueCastle
{
	class MapScreen
	{
	public:
		~MapScreen();
		bool LoadContent(const char *assetsDir);
		// Rooms already seen in this castle (index = castle room)
		void Reset(size_t numRooms) { m_visited.assign(numRooms, false); }
		// Marks the room holding (x, y) as visited
		void Visit(const Levels::Castle &castle, float x, float y);
		// Restores the visited flags of a locked castle (SaveGameManager
		// line 1946: MapRoomsUnveiled)
		void SetVisited(const std::vector<uint8_t> &visited) { m_visited.assign(visited.begin(), visited.end()); }
		void GetVisited(std::vector<uint8_t> &out) const { out.assign(m_visited.begin(), m_visited.end()); }
		size_t VisitedSize() const { return m_visited.size(); }
		// The castle's chests, for the map icons (MapObj.AddRoom). kind: one
		// of MapChest::LOCKED / FAIRY / FAIRY_INVISIBLE / OPENED.
		struct MapChest
		{
			enum Kind { LOCKED, FAIRY, FAIRY_INVISIBLE, OPENED };
			float x, y;   // castle coordinates
			uint8_t kind;
		};
		void SetChests(const std::vector<MapChest> &chests) { m_chests = chests; }
		// The spelunker (Banker2): every room is on the map from the start,
		// and no chest icons (MapObj.AddRoom line 160)
		void SetSpelunker(bool on) { m_spelunker = on; }
		void Open(float playerX, float playerY);
		/* A teleporter pad's Up: the selection screen over the map
		 * (MapScreen.IsTeleporter), destinations in visit order with the
		 * CastleEntrance first, like the icons the C# adds on visit. */
		void OpenTeleporter(float playerX, float playerY, const Levels::Castle &castle);
		/* After Update() closed a teleporter screen: the chosen room
		 * (MapScreen.cs line 344, MapObj.TeleportPlayer). */
		bool TeleportConfirmed(int &roomIndex);
		// false once closed
		bool Update(uint32_t buttons, uint32_t pressed, double dt);
		// alzheimers: the rooms are forgotten, "?????" instead (MapScreen.cs line 259)
		void Draw(uint32_t *frame, const Levels::Castle &castle, bool alzheimers = false);
		// The minimap in the top right corner of the play area
		void DrawMini(uint32_t *frame, const Levels::Castle &castle, float playerX, float playerY, double dt);

	private:
		std::vector<bool> m_visited;
		std::vector<MapChest> m_chests;
		bool m_spelunker = false;
		DS2DEngine::Camera2D m_camera;
		DS2DEngine::SpriteObj *m_door = nullptr, *m_boss = nullptr, *m_player = nullptr;
		DS2DEngine::SpriteObj *m_chestLocked = nullptr, *m_chestFairy = nullptr,
			*m_chestOpened = nullptr, *m_bonus = nullptr, *m_teleporter = nullptr,
			*m_legendPlate = nullptr, *m_titleText = nullptr;
		/* The teleporter selection (IsTeleporter): destination room indices
		 * and the selected one. */
		bool m_isTeleporter = false;
		int m_selectedTeleporter = 0;
		int m_teleportChoice = -1;
		std::vector<int> m_dests;
		void DrawTeleporters(const Levels::Castle &castle);
		std::vector<DS2DEngine::SpriteObj *> m_rooms;   // MapRoom{w}x{h}, by "w*10+h"
		Text::Font m_font;
		float m_playerX = 0, m_playerY = 0;   // castle coordinates
		float m_offsetX = 0, m_offsetY = 0;   // map pan, in map pixels
		double m_time = 0;

		DS2DEngine::SpriteObj *m_miniBG = nullptr;
		std::vector<uint32_t> m_scratch;   // the minimap is drawn here, then clipped

		DS2DEngine::SpriteObj *RoomSprite(int w, int h);
		void DrawRooms(const Levels::Castle &castle, float ox, float oy, float k);
	};
}
