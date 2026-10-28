// The title screen (TitleScreen.cs): the red sky (TitleBG tinted red),
// the turning small clouds, the castle, the drifting large clouds, the
// logo fading and sliding in, and the pulsing "A  Continuer votre quête"
// (or "Choisir votre héritier" / "Préparer votre héritage"). The god rays
// and the profile/options/credits icons are left out. Its three sheets
// (37 MB) are loaded by Open and freed by Close. A or Start: continue.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	class TitleScreen
	{
	public:
		~TitleScreen() { Close(); }
		// startTextID: LOC_ID_TITLE_SCREEN_2/3/4 (InitializeStartingText)
		bool Open(const char *assetsDir, const char *startTextID);
		void Close();
		// true once A or Start is pressed
		bool Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		std::string m_assets;
		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // the 1320x720 screen at zoom 0.5
		DS2DEngine::SpriteObj *m_bg = nullptr, *m_castle = nullptr, *m_logo = nullptr;
		DS2DEngine::SpriteObj *m_small[5] = {}, *m_large[4] = {};
		std::string m_startText;
		double m_time = 0;
		bool m_open = false;
	};
}
