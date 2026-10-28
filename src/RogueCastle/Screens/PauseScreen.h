// The pause screen (PauseScreen.cs), opened with Start in the castle: the
// game dimmed, the "Jeu en pause" title art, and an info plate over the
// hero (class, strength) and over every enemy on screen (level, strength,
// health). The plates are plain boxes: GameOverStatPlate and DialogBox are
// not in the converted sheets. Start or B: resume.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <cstdint>
#include <string>
#include <vector>

namespace RogueCastle
{
	class PlayerObj;
	class RoomObj;

	class PauseScreen
	{
	public:
		~PauseScreen();
		bool LoadContent(const char *assetsDir);
		// false once closed
		bool Update(uint32_t pressed);
		// heroName: the lineage name; className: the translated class
		void Draw(uint32_t *frame, DS2DEngine::Camera2D *camera, PlayerObj *player, RoomObj *room,
			const std::string &heroName, const char *className);

	private:
		Text::Font m_font;
		DS2DEngine::Camera2D m_screen;   // zoom 1: world = screen
		DS2DEngine::SpriteObj *m_title = nullptr;

		void Plate(uint32_t *frame, float cx, float bottom, const std::string &name,
			const std::vector<std::pair<std::string, std::string>> &items);
	};
}
