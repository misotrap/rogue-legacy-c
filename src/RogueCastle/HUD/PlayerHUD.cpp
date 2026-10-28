#include "PlayerHUD.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../platform.h"

#include <cstdio>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const uint32_t BLACK = 0xFF000000u, WHITE = 0xFFFFFFFFu;
		const int MAX_BAR_LENGTH = 360;   // m_maxBarLength

		SpriteObj *Make(const char *name)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			return s;
		}

		float TextWidth(const Text::Font &font, const char *s)
		{
			float w = 0, h = 0;
			font.Measure(s, w, h);
			return w;
		}
	}

	PlayerHUD::~PlayerHUD()
	{
		for (Bar *b : { &m_hp, &m_mp })
		{
			delete b->left; delete b->mid; delete b->right; delete b->fill;
		}
		delete m_lvl;
		delete m_coin;
	}

	bool PlayerHUD::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		ok = m_goldFont.Load(assetsDir, "GoldFont") && ok;
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("HPBarLeft_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "mapSpritesheetBig") && ok;
		m_hp = { Make("HPBarLeft_Sprite"), Make("HPBarMid_Sprite"), Make("HPBarRight_Sprite"), Make("HPBar_Sprite") };
		m_mp = { Make("MPBarLeft_Sprite"), Make("MPBarMid_Sprite"), Make("MPBarRight_Sprite"), Make("MPBar_Sprite") };
		m_lvl = Make("PlayerHUDLvlText_Sprite");
		m_coin = Make("PlayerUICoin_Sprite");
		// Zoom 1 centred on (320, 240): world coordinates are screen pixels
		m_camera.Position(Vector2(SCREEN_W / 2, SCREEN_H / 2));
		m_camera.Zoom(1.0f);
		return ok;
	}

	// UpdatePlayerHP/MP: the frame grows by 1 px per 5 bonus points over
	// the base (88 px, at most 360); the fill sits 2 px in and covers
	// (frame width - 8) * percent. The fill is drawn first, the frame over it.
	void PlayerHUD::DrawBar(Bar &b, float x, float y, int value, int max, int base, float fillDy)
	{
		if (max < 1) max = 1;
		if (value < 0) value = 0;
		if (value > max) value = max;
		if (base <= 0 || base > max) base = max;
		int width = 88 + (max - base) / 5;
		if (width > MAX_BAR_LENGTH) width = MAX_BAR_LENGTH;
		float midScale = (width - 28 - 28) / 32.0f;
		float frameWidth = 64 + 32 * midScale;

		b.fill->ScaleX(1);
		b.fill->ScaleX((frameWidth - 8) / b.fill->Width() * (value / (float) max));
		b.fill->Position(Vector2(x + 2, y + fillDy));
		if (value > 0) b.fill->Draw(&m_camera);

		b.left->Position(Vector2(x, y));
		b.mid->ScaleX(midScale);
		b.mid->Position(Vector2(x + 32, y));
		b.right->Position(Vector2(x + 32 + 32 * midScale, y));
		b.left->Draw(&m_camera);
		b.mid->Draw(&m_camera);
		b.right->Draw(&m_camera);
	}

	void PlayerHUD::Draw(uint32_t *frame, const HUDValues &v)
	{
		char buf[64];
		gfx_begin(frame);
		if (m_hp.fill == nullptr) return;

		// HP frame (34 px) over MP frame (24 px): together they fill the band
		const float x = 8, hpY = 1, mpY = 35;
		// CIP (PlayerHUDObj.cs line 323): the health frame stays empty
		DrawBar(m_hp, x, hpY, v.cip ? 0 : v.health, v.maxHealth, v.baseHealth, 6);
		DrawBar(m_mp, x, mpY, v.mana, v.maxMana, v.baseMana, 7);
		snprintf(buf, sizeof(buf), "%d/%d", v.health, v.maxHealth);
		if (!v.cip) m_font.DrawOutlined(frame, buf, x + 6, hpY + 6, 0.26f, WHITE, BLACK, 1);
		snprintf(buf, sizeof(buf), "%d/%d", v.mana, v.maxMana);
		m_font.DrawOutlined(frame, buf, x + 6, mpY + 4, 0.22f, WHITE, BLACK, 1);

		// "Lv" plate with the level, then the coin with the gold (top right)
		snprintf(buf, sizeof(buf), "%d", v.gold);
		float coinX = SCREEN_W - 10 - TextWidth(m_goldFont, buf) - 28;
		m_coin->Position(Vector2(coinX, 4));
		m_coin->Draw(&m_camera);
		m_goldFont.Draw(frame, buf, coinX + 28, 6);

		snprintf(buf, sizeof(buf), "%d", v.level);
		float lvlX = coinX - 24 - TextWidth(m_goldFont, buf) - 31;
		m_lvl->Position(Vector2(lvlX, 8));
		m_lvl->Draw(&m_camera);
		m_goldFont.Draw(frame, buf, lvlX + 31, 6);
	}
}
