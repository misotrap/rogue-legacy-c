#include "TitleScreen.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cmath>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const char *SHEETS[] = { "miscSpritesheet", "getItemScreenSpritesheet", "titleScreen3Spritesheet" };
		const uint32_t BLACK = 0xFF000000u;

		SpriteObj *Make(const char *name, float x, float y)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			s->Position(Vector2(x, y));
			return s;
		}

		float QuadInOut(float t)
		{
			if (t >= 1) return 1;
			return t < 0.5f ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t);
		}
	}

	bool TitleScreen::Open(const char *assetsDir, const char *startTextID)
	{
		Close();
		m_assets = assetsDir;
		bool ok = m_font.Loaded() || m_font.Load(assetsDir, "Junicode");
		for (const char *sheet : SHEETS)
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, sheet) && ok;

		m_bg = Make("TitleBG_Sprite", 0, 0);
		m_bg->Scale(Vector2(1320.0f / m_bg->Width(), 720.0f / m_bg->Height()));
		m_bg->TextureColor(Color(255, 0, 0));
		m_castle = Make("TitleCastle_Sprite", 0, 0);
		m_castle->Scale(Vector2(2, 2));
		m_castle->Position(Vector2(1320 / 2 - 30, 720 - m_castle->Height() / 2));
		static const char *small[5] = { "TitleSmallCloud1_Sprite", "TitleSmallCloud2_Sprite", "TitleSmallCloud3_Sprite",
			"TitleSmallCloud4_Sprite", "TitleSmallCloud5_Sprite" };
		for (int i = 0; i < 5; i++) m_small[i] = Make(small[i], 1320 / 2, 0);
		m_large[0] = Make("TitleLargeCloud1_Sprite", 0, 0);
		m_large[0]->Y(720 - m_large[0]->Height());
		m_large[1] = Make("TitleLargeCloud2_Sprite", 1320 / 3, 0);
		m_large[1]->Y(720 - m_large[1]->Height() + 130);
		m_large[2] = Make("TitleLargeCloud1_Sprite", 1320 / 3 * 2, 0);
		m_large[2]->Y(720 - m_large[2]->Height() + 50);
		m_large[3] = Make("TitleLargeCloud2_Sprite", 1320, 0);
		m_large[3]->Y(720 - m_large[3]->Height());
		m_large[3]->Flip(SpriteEffects::FlipHorizontally);
		m_logo = Make("TitleLogo_Sprite", 1320 / 2, 720 / 2 - 50);
		// TitleScreen.LoadContent: the options and credits icons, level with
		// the profile card (720 - its height)
		m_optionsIcon = Make("TitleOptionsIcon_Sprite", 1320 - 48 * 2 * 2, 720 - 50 * 2);
		m_creditsIcon = Make("TitleCreditsIcon_Sprite", 1320 - 48 * 2 * 2 + 120, 720 - 50 * 2);
		for (SpriteObj *s : { m_optionsIcon, m_creditsIcon }) if (s != nullptr) s->Scale(Vector2(2, 2));

		m_startText = Text::Loc::Get(startTextID);
		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		m_time = 0;
		m_open = true;
		return ok;
	}

	void TitleScreen::Close()
	{
		if (!m_open) return;
		delete m_bg; delete m_castle; delete m_logo; delete m_optionsIcon; delete m_creditsIcon;
		m_bg = m_castle = m_logo = m_optionsIcon = m_creditsIcon = nullptr;
		for (SpriteObj *&s : m_small) { delete s; s = nullptr; }
		for (SpriteObj *&s : m_large) { delete s; s = nullptr; }
		for (const char *sheet : SHEETS) SpriteSystem::SpriteLibrary::FreeSheet(sheet);
		m_open = false;
	}

	bool TitleScreen::Update(uint32_t pressed, double dt)
	{
		if (!m_open) return true;
		m_time += dt;
		const float e = (float) dt;
		static const float spin[5] = { 1.8f, 1.2f, 3, -0.6f, -1.8f };
		for (int i = 0; i < 5; i++) m_small[i]->Rotation(m_small[i]->Rotation() + spin[i] * e);
		// The large clouds drift and wrap around
		m_large[1]->X(m_large[1]->X() + 2.4f * e);
		if (m_large[1]->X() > 1320 + m_large[1]->Width() / 2) m_large[1]->X(0 - m_large[1]->Width() / 2);
		m_large[2]->X(m_large[2]->X() - 3 * e);
		if (m_large[2]->X() < 0 - m_large[2]->Width() / 2) m_large[2]->X(1320 + m_large[2]->Width() / 2);
		// The logo fades in over 2 s and eases down to y 360 over 3 s
		float t = (float) m_time;
		m_logo->Opacity(t >= 2 ? 1 : t / 2);
		m_logo->Y(310 + 50 * QuadInOut(t / 3));
		return (pressed & (BTN_A | BTN_START)) != 0;
	}

	void TitleScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		gfx_clear(BLACK);
		if (!m_open) return;
		m_bg->Draw(&m_camera);
		m_small[0]->Draw(&m_camera);
		m_small[2]->Draw(&m_camera);
		m_small[3]->Draw(&m_camera);
		m_castle->Draw(&m_camera);
		m_small[1]->Draw(&m_camera);
		m_small[4]->Draw(&m_camera);
		for (SpriteObj *s : m_large) s->Draw(&m_camera);
		m_logo->Draw(&m_camera);
		// The 1320x720 screen ends at the letterbox: crop what hangs out
		gfx_fill(0, 0, SCREEN_W, 60, BLACK);
		gfx_fill(0, 420, SCREEN_W, SCREEN_H - 420, BLACK);

		// "[A]  Continuer votre quête", pulsing like |sin(time)|
		float pulse = (float) std::fabs(std::sin(m_time));
		uint32_t c = (uint32_t) (55 + 200 * pulse);
		uint32_t colour = 0xFF000000u | c << 16 | c << 8 | c;
		// The options and credits icons and their keys above them
		const char *keys[] = { "SELECT", "Y" };
		SpriteObj *icons[] = { m_optionsIcon, m_creditsIcon };
		for (int i = 0; i < 2; i++)
		{
			if (icons[i] == nullptr) continue;
			icons[i]->Draw(&m_camera);
			float kx = icons[i]->X() * 0.5f - 10, ky = (icons[i]->Bounds().Top() - 10) * 0.5f + 60 - 14;
			m_font.DrawOutlined(frame, keys[i], kx, ky, 0.3f, 0xFFFFFFFFu, BLACK, 1, Text::Align::Centre);
		}
		std::string text = "A  " + m_startText;
		m_font.DrawOutlined(frame, text.c_str(), SCREEN_W / 2, 240 + 100 - 8, 0.4f, colour, BLACK, 1, Text::Align::Centre);
	}
}
