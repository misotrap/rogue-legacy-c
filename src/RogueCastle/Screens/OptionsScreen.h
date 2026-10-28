// The options (OptionsScreen.cs), the entries that make sense on the
// console: music volume, sound effects volume, back to the title screen
// (in the castle only) and quit. The original plate, title, selection bar
// and volume bars, from uiExtraSpritesheet. Up/Down choose, Left/Right
// change a volume, A runs an entry, B or START leave. The volumes are kept
// in config.txt next to the save (GameConfig).
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	class OptionsScreen
	{
	public:
		enum class Result { Open, Closed, BackToTitle, Quit };

		~OptionsScreen();
		// configPath: e.g. "config.txt"; applies the saved volumes
		bool LoadContent(const char *assetsDir, const char *configPath);
		void Open(bool inCastle);
		Result Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		enum Entry { MUSIC, SFX, BACK_TO_TITLE, QUIT, COUNT };
		std::string m_configPath;
		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // the 1320x720 screen at zoom 0.5
		DS2DEngine::ObjContainer *m_plate = nullptr;
		DS2DEngine::SpriteObj *m_title = nullptr, *m_bar = nullptr, *m_volumeBG = nullptr, *m_volume = nullptr;
		float m_music = 1, m_sfx = 1;
		int m_selected = 0;
		bool m_inCastle = false;

		bool Shown(int entry) const { return entry != BACK_TO_TITLE || m_inCastle; }
		void Apply();
		void SaveConfig() const;
	};
}
