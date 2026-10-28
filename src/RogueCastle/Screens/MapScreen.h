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
		void Open(float playerX, float playerY);
		// false once closed
		bool Update(uint32_t buttons, uint32_t pressed, double dt);
		void Draw(uint32_t *frame, const Levels::Castle &castle);
		// The minimap in the top right corner of the play area
		void DrawMini(uint32_t *frame, const Levels::Castle &castle, float playerX, float playerY, double dt);

	private:
		std::vector<bool> m_visited;
		DS2DEngine::Camera2D m_camera;
		DS2DEngine::SpriteObj *m_door = nullptr, *m_boss = nullptr, *m_player = nullptr;
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
