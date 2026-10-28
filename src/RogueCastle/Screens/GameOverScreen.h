// The death screen (GameOverScreen.cs), simplified: the last game frame
// fades to black (BackBufferOpacity, 0.5 s), the spotlight comes on at 1 s,
// the hero's ghost rises at 3 s, and the dialogue plate shows
// "<hero> a été tué", a random game hint and "<hero>, dernières paroles".
// "Appuyez sur A pour continuer" appears at 4 s. The camera zoom on the
// body, the killer's name and the enemies walking in are left out (the
// killer is not tracked yet). The art is in the small uiExtraSpritesheet
// (tools/make_subsheet.py), shared with the dialogue box and the courtyard.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <cstdint>
#include <string>
#include <vector>

namespace RogueCastle
{
	class GameOverScreen
	{
	public:
		~GameOverScreen() { Close(); }
		// lastFrame: the castle frame at the moment of death (SCREEN_W*SCREEN_H)
		bool Open(const char *assetsDir, const std::vector<uint32_t> &lastFrame, const std::string &heroName, bool isFemale);
		void Close();
		// true once A is pressed after the sequence
		bool Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // the 1320x720 screen at zoom 0.5
		DS2DEngine::SpriteObj *m_spotlight = nullptr, *m_ghost = nullptr, *m_plate = nullptr;
		std::vector<uint32_t> m_lastFrame;
		std::string m_slain, m_hint, m_partingTitle, m_continue;
		double m_time = 0;
		bool m_open = false;
	};
}
