#include "PlayerHUD.h"

#include "../../platform.h"

#include <cstdio>

namespace RogueCastle
{
	namespace
	{
		const uint32_t BLACK = 0xFF000000u, WHITE = 0xFFFFFFFFu, GOLD = 0xFFF0C850u;

		void Bar(int x, int y, int w, int h, int value, int max, uint32_t fill, uint32_t back)
		{
			if (max < 1) max = 1;
			if (value < 0) value = 0;
			if (value > max) value = max;
			gfx_fill(x - 1, y - 1, w + 2, h + 2, BLACK);
			gfx_fill(x, y, w, h, back);
			gfx_fill(x, y, w * value / max, h, fill);
		}
	}

	bool PlayerHUD::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		return m_goldFont.Load(assetsDir, "GoldFont") && ok;
	}

	void PlayerHUD::Draw(uint32_t *frame, const HUDValues &v)
	{
		char buf[64];
		gfx_begin(frame);
		// Health (red) and mana (blue), with "value/max" on each bar
		Bar(8, 8, 180, 14, v.health, v.maxHealth, 0xFFC83232u, 0xFF3A1414u);
		Bar(8, 28, 140, 10, v.mana, v.maxMana, 0xFF3C64DCu, 0xFF141E3Au);
		snprintf(buf, sizeof(buf), "%d/%d", v.health, v.maxHealth);
		m_font.DrawOutlined(frame, buf, 12, 6, 0.26f, WHITE, BLACK, 1);
		snprintf(buf, sizeof(buf), "%d/%d", v.mana, v.maxMana);
		m_font.DrawOutlined(frame, buf, 12, 24, 0.22f, WHITE, BLACK, 1);
		// Hero name and level under the bars
		snprintf(buf, sizeof(buf), "%s  -  Niv. %d", v.heroName.c_str(), v.level);
		m_font.DrawOutlined(frame, buf, 8, 41, 0.24f, GOLD, BLACK, 1);
		// Gold, top right (GoldFont only has digits)
		snprintf(buf, sizeof(buf), "%d", v.gold);
		if (m_goldFont.Loaded()) m_goldFont.Draw(frame, buf, SCREEN_W - 10, 10, 1.0f, WHITE, Text::Align::Right);
		else m_font.DrawOutlined(frame, buf, SCREEN_W - 10, 10, 0.3f, GOLD, BLACK, 1, Text::Align::Right);
	}
}
