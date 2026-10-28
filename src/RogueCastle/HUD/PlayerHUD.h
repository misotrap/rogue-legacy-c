// In-game HUD (PlayerHUDObj.cs) with the original art from
// mapSpritesheetBig: the HP/MP bar frames (left, stretched middle, right)
// over their fills, the "Lv" plate with the level, the coin with the gold.
// Drawn at scale 1 in the top letterbox band (y 0..60), so it never covers
// the play area. The game passes the values each frame.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <string>

namespace RogueCastle
{
	struct HUDValues
	{
		int health = 0, maxHealth = 1, baseHealth = 0;  // base 0: = max
		int mana = 0, maxMana = 1, baseMana = 0;
		int gold = 0;
		bool cip = false;       // Congenital insensitivity to pain: no health bar
		int level = 0;          // PlayerStats.CurrentLevel (manor level)
		std::string heroName;   // e.g. "Sieur Mark II"
	};

	class PlayerHUD
	{
	public:
		~PlayerHUD();
		bool LoadContent(const char *assetsDir);
		void Draw(uint32_t *frame, const HUDValues &v);

	private:
		struct Bar { DS2DEngine::SpriteObj *left = nullptr, *mid = nullptr, *right = nullptr, *fill = nullptr; };
		Text::Font m_font, m_goldFont;
		DS2DEngine::Camera2D m_camera;   // zoom 1: world = screen
		Bar m_hp, m_mp;
		DS2DEngine::SpriteObj *m_lvl = nullptr, *m_coin = nullptr;

		void DrawBar(Bar &b, float x, float y, int value, int max, int base, float fillDy);
	};
}
