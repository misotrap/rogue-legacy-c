#include "CreditsScreen.h"

#include "../../Audio/Audio.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u, GOLD = 0xFFFFD700u;
		const float SPEED = 30;   // m_scrollDuration over m_scrollDistance, in screen pixels/s
		const float LINE = 16, GAP = 14;

		// InitializeCredits: "@ID" lines are the gold titles
		const char *const CREDITS[] = {
			"@LOC_ID_CREDITS_SCREEN_6", "Cellar Door Games", "",
			"@LOC_ID_CREDITS_SCREEN_7", "Teddy Lee", "",
			"@LOC_ID_CREDITS_SCREEN_8", "Kenny Lee", "",
			"@LOC_ID_CREDITS_SCREEN_9", "Marie-Christine Bourdua", "",
			"@LOC_ID_CREDITS_SCREEN_10", "Glauber Kotaki", "",
			"@LOC_ID_CREDITS_SCREEN_11", "Gordon McGladdery (A Shell in the Pit)", "",
			"@LOC_ID_CREDITS_SCREEN_12", "Judson Cowan (Tettix)", "",
			"@LOC_ID_CREDITS_SCREEN_13", "Benny Lee", "",
			"@LOC_ID_CREDITS_SCREEN_25", "David Man", "",
			"@LOC_ID_CREDITS_SCREEN_14", "Alessio Mellina", "",
			"@LOC_ID_CREDITS_SCREEN_15", "John Won", "",
			"@LOC_ID_CREDITS_SCREEN_16", "Ryan & Michelle Lee", "",
			"@LOC_ID_CREDITS_SCREEN_17", "Jenny Lee", "",
			"@LOC_ID_CREDITS_SCREEN_20", "Ethan 'flibitijibibo' Lee", "",
			"@LOC_ID_CREDITS_SCREEN_23", "Babel Media", "",
		};
	}

	bool CreditsScreen::LoadContent(const char *assetsDir)
	{
		return m_font.Load(assetsDir, "Junicode");
	}

	void CreditsScreen::Open(int numChildren)
	{
		m_children = numChildren;
		m_time = 0;
		m_height = 0;
		for (const char *c : CREDITS)
			m_height += *c ? LINE : GAP;
		Audio::PlayMusic("CreditsSong");
	}

	bool CreditsScreen::Update(uint32_t pressed, double dt)
	{
		m_time += dt;
		// The list has to be past before A leaves (m_allowExit)
		return m_time * SPEED > m_height + SCREEN_H / 2 && (pressed & (BTN_A | BTN_B | BTN_START));
	}

	void CreditsScreen::Draw(uint32_t *frame)
	{
		gfx_begin(frame);
		// The night sky: dark blue fading to black at the top
		for (int y = 0; y < SCREEN_H; y++)
		{
			int b = 20 + 50 * y / SCREEN_H;
			gfx_fill(0, y, SCREEN_W, 1, 0xFF000000u | (uint32_t) (b / 3) << 16 | (uint32_t) (b / 2) << 8 | (uint32_t) b);
		}

		float y = SCREEN_H - (float) m_time * SPEED;
		for (const char *c : CREDITS)
		{
			if (!*c) { y += GAP; continue; }
			if (y > -LINE && y < SCREEN_H)
			{
				bool title = c[0] == '@';
				m_font.DrawOutlined(frame, title ? Text::Loc::Get(c + 1) : c, SCREEN_W / 2, y,
					title ? 0.26f : 0.32f, title ? GOLD : WHITE, BLACK, 1, Text::Align::Centre);
			}
			y += LINE;
		}

		// The end: thanks and the number of heirs, staying in the middle
		float endY = y + 40;
		if (endY < SCREEN_H / 2 - 30) endY = SCREEN_H / 2 - 30;
		if (endY < SCREEN_H)
		{
			m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_CREDITS_SCREEN_1"), SCREEN_W / 2, endY, 0.5f, WHITE, BLACK, 2, Text::Align::Centre);
			std::string kids = std::string(Text::Loc::Get("LOC_ID_CREDITS_SCREEN_2")) + " : " + std::to_string(m_children);
			m_font.DrawOutlined(frame, kids.c_str(), SCREEN_W / 2, endY + 36, 0.3f, GOLD, BLACK, 1, Text::Align::Centre);
		}
		if (m_time * SPEED > m_height + SCREEN_H / 2)
		{
			std::string leave = Text::Loc::Get("LOC_ID_CREDITS_SCREEN_4_NEW");
			size_t at = leave.find("[Input:0]");
			if (at != std::string::npos) leave.replace(at, 9, "A");
			m_font.DrawOutlined(frame, leave.c_str(), SCREEN_W - 12, 400, 0.24f, WHITE, BLACK, 1, Text::Align::Right);
		}
	}
}
