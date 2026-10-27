// The manor (SkillScreen.cs): the castle that grows with each upgrade, the
// skill icons at their original places, the selection halo, and a panel on
// the right with the selected skill (name, level, current and next bonus,
// description, price) and the gold. Arrows move along the original links,
// B buys or upgrades, START leaves the manor.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"
#include "../Save/SaveGame.h"

#include <vector>

namespace RogueCastle
{
	class ManorScreen
	{
	public:
		ManorScreen();
		~ManorScreen();

		bool LoadContent(const char *assetsDir);
		void Open(Save::GameState &state);
		// Returns true when the player leaves the manor (START)
		bool Update(uint32_t buttons, uint32_t pressed, double elapsedSeconds);
		void Draw(uint32_t *frame);

		// The skill under the cursor
		Skills::SkillType Selected() const;

	private:
		Save::GameState *m_state = nullptr;
		DS2DEngine::Camera2D m_camera;
		DS2DEngine::ObjContainer *m_manor = nullptr;
		std::vector<DS2DEngine::SpriteObj *> m_icons; // one per Skills::SKILLS entry
		DS2DEngine::SpriteObj *m_halo = nullptr;
		Text::Font m_font, m_titleFont;
		int m_x = 0, m_y = 0;          // selected grid cell (column, row)
		double m_flash = 0;            // "not enough gold" feedback

		void Refresh();                // icons and manor pieces from the skill levels
		void Select(int x, int y);
		void Clear();
	};
}
