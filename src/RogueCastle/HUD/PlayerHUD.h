// In-game HUD (PlayerHUDObj.cs, simplified for the 640x480 screen): health
// and mana bars with their numbers, gold, hero level, and the hero's name.
// Drawn in the top letterbox band (y 0..60), so it never covers the play
// area. The game passes the values each frame.
#pragma once

#include "../../Text/Font.h"

#include <string>

namespace RogueCastle
{
	struct HUDValues
	{
		int health = 0, maxHealth = 1;
		int mana = 0, maxMana = 1;
		int gold = 0;
		int level = 0;          // PlayerStats.CurrentLevel (manor level)
		std::string heroName;   // e.g. "Sieur Mark II"
	};

	class PlayerHUD
	{
	public:
		bool LoadContent(const char *assetsDir);
		void Draw(uint32_t *frame, const HUDValues &v);

	private:
		Text::Font m_font, m_goldFont;
	};
}
