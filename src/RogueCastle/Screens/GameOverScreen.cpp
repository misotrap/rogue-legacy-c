#include "GameOverScreen.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstdio>
#include <cstdlib>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const char *SHEET = "miscSpritesheet";
		const int NUM_HINTS = 44;   // GameEV.GAME_HINTS: LOC_ID_HINT_1..44
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u;
		const float GHOST_Y = 280;
		const uint32_t HINT_COLOUR = 0xFFFFFE80u;   // Color(255, 254, 128)

		SpriteObj *Make(const char *name, float x, float y)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			s->Position(Vector2(x, y));
			return s;
		}

		float Clamp01(double v) { return v < 0 ? 0 : v > 1 ? 1 : (float) v; }

		// frame = last * (1 - k), k in 0..1
		void Darken(uint32_t *frame, const std::vector<uint32_t> &last, float k)
		{
			uint32_t m = (uint32_t) ((1 - k) * 256);
			for (size_t i = 0; i < last.size(); i++)
			{
				uint32_t c = last[i];
				frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * m >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * m >> 8) & 0xFF00u);
			}
		}
	}

	bool GameOverScreen::Open(const char *assetsDir, const std::vector<uint32_t> &lastFrame, const std::string &heroName)
	{
		Close();
		bool ok = m_font.Loaded() || m_font.Load(assetsDir, "Junicode");
		ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, SHEET) && ok;
		m_lastFrame = lastFrame;
		m_lastFrame.resize((size_t) SCREEN_W * SCREEN_H, BLACK);

		m_camera.Position(Vector2(660, 360));
		m_camera.Zoom(0.5f);
		// The light cone points down onto the body (the screen centre)
		m_spotlight = Make("GameOverSpotlight_Sprite", 1320 / 2, 40 + 150);
		m_spotlight->Rotation(90);
		m_ghost = Make("PlayerGhost_Sprite", 1320 / 2 - 36, GHOST_Y);
		m_ghost->AnimationDelay(1 / 10.0f);
		m_ghost->PlayAnimation(true);
		m_plate = Make("DialogBox_Sprite", 1320 / 2, 610);

		std::string name = heroName;   // "Sieur Lee " when there is no numeral
		while (!name.empty() && name.back() == ' ') name.pop_back();
		m_slain = Text::Loc::Format("LOC_ID_GAME_OVER_SCREEN_7_NEW", name);
		char id[32];
		snprintf(id, sizeof(id), "LOC_ID_HINT_%d", 1 + rand() % NUM_HINTS);
		m_hint = Text::Loc::Get(id);
		m_partingTitle = Text::Loc::Format("LOC_ID_GAME_OVER_SCREEN_8_NEW", name);
		m_continue = Text::Loc::Get("LOC_ID_GAME_OVER_SCREEN_1_NEW");
		size_t at = m_continue.find("[Input:0]");
		if (at != std::string::npos) m_continue.replace(at, 9, "A");
		m_time = 0;
		m_open = true;
		return ok;
	}

	void GameOverScreen::Close()
	{
		if (!m_open) return;
		delete m_spotlight; delete m_ghost; delete m_plate;
		m_spotlight = m_ghost = m_plate = nullptr;
		SpriteSystem::SpriteLibrary::FreeSheet(SHEET);
		m_lastFrame.clear();
		m_lastFrame.shrink_to_fit();
		m_open = false;
	}

	bool GameOverScreen::Update(uint32_t pressed, double dt)
	{
		if (!m_open) return true;
		m_time += dt;
		// The ghost: 0.4 opacity at 3 s, rises 150 over 2 s, gone at 4.5 s
		double g = m_time - 3;
		m_ghost->Opacity(g < 0 ? 0 : g < 0.5 ? 0.8f * (float) g : g < 1 ? 0.4f : g < 1.5 ? 0.4f * (float) (1.5 - g) / 0.5f : 0);
		m_ghost->Y(GHOST_Y - 150 * Clamp01(g / 2));
		return m_time > 4 && (pressed & (BTN_A | BTN_START)) != 0;
	}

	void GameOverScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		if (!m_open) { gfx_clear(BLACK); return; }
		Darken(frame, m_lastFrame, Clamp01(m_time / 0.5));
		if (m_time >= 1) m_spotlight->Draw(&m_camera);
		if (m_time >= 3) m_ghost->Draw(&m_camera);

		// The dialogue plate fades in with the spotlight
		if (m_time >= 1)
		{
			m_plate->Opacity(Clamp01((m_time - 1) / 0.5));
			m_plate->Draw(&m_camera);
			const float cx = SCREEN_W / 2, top = 365 - 45;
			m_font.DrawOutlined(frame, m_slain.c_str(), cx, top + 8, 0.42f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, m_hint.c_str(), cx, top + 36, 0.3f, HINT_COLOUR, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, m_partingTitle.c_str(), cx + 220, top + 66, 0.22f, WHITE, BLACK, 1, Text::Align::Right);
		}
		if (m_time > 4)
			m_font.DrawOutlined(frame, m_continue.c_str(), SCREEN_W - 20, 70, 0.3f, WHITE, BLACK, 1, Text::Align::Right);
	}
}
