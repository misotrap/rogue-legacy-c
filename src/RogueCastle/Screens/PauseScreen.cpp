#include "PauseScreen.h"

#include "EnemyNames.h"
#include "../GameObjects/EnemyObjs/EnemyObj.h"
#include "../GameObjects/Player/PlayerObj.h"
#include "../GameObjects/RoomObjs/RoomObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstring>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u, GOLD = 0xFFF0C850u;
		const float TEXT = 0.24f;

		const char *EnemyName(const std::string &name)
		{
			for (const EnemyNameLoc &e : ENEMY_NAMES)
				if (name == e.name) return Text::Loc::Get(e.locID);
			return name.c_str();
		}

		// The level screen at 40% (the original pause dims the game)
		void Dim(uint32_t *frame)
		{
			for (size_t i = 0; i < (size_t) SCREEN_W * SCREEN_H; i++)
			{
				uint32_t c = frame[i];
				frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * 102 >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * 102 >> 8) & 0xFF00u);
			}
		}

		float TextWidth(const Text::Font &font, const std::string &s)
		{
			float w = 0, h = 0;
			font.Measure(s.c_str(), w, h);
			return w * TEXT;
		}
	}

	PauseScreen::~PauseScreen()
	{
		delete m_title;
	}

	bool PauseScreen::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("GamePausedTitleText_Sprite_FR"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "language3Spritesheet") && ok;
		// Game.ChangeBitmapLanguage: French only for now
		m_title = new SpriteObj("GamePausedTitleText_Sprite_FR");
		m_title->ForceDraw(true);
		m_screen.Position(Vector2(SCREEN_W / 2, SCREEN_H / 2));
		m_screen.Zoom(1.0f);
		return ok;
	}

	bool PauseScreen::Update(uint32_t pressed)
	{
		return (pressed & (BTN_START | BTN_B)) == 0;
	}

	// PauseInfoObj: the name on its plate, then "title  data" lines,
	// centred on cx with the bottom edge at 'bottom'
	void PauseScreen::Plate(uint32_t *frame, float cx, float bottom, const std::string &name,
		const std::vector<std::pair<std::string, std::string>> &items)
	{
		const float line = 13;
		float titleW = 0, dataW = 0;
		for (const auto &it : items)
		{
			float tw = TextWidth(m_font, it.first), dw = TextWidth(m_font, it.second);
			if (tw > titleW) titleW = tw;
			if (dw > dataW) dataW = dw;
		}
		float w = titleW + dataW + 20, nameW = TextWidth(m_font, name) + 20;
		if (nameW > w) w = nameW;
		float h = line * (items.size() + 1) + 8;
		int x = (int) (cx - w / 2), y = (int) (bottom - h);
		if (x < 2) x = 2;
		if (x + w > SCREEN_W - 2) x = (int) (SCREEN_W - 2 - w);
		if (y < 62) y = 62;
		gfx_fill(x - 1, y - 1, (int) w + 2, (int) h + 2, GOLD);
		gfx_fill(x, y, (int) w, (int) h, 0xFF141018u);

		float ty = y + 3;
		m_font.DrawOutlined(frame, name.c_str(), x + w / 2, ty, TEXT, GOLD, BLACK, 1, Text::Align::Centre);
		for (const auto &it : items)
		{
			ty += line;
			m_font.DrawOutlined(frame, it.first.c_str(), x + 8, ty, TEXT, WHITE, BLACK, 1);
			m_font.DrawOutlined(frame, it.second.c_str(), x + 12 + titleW, ty, TEXT, WHITE, BLACK, 1);
		}
	}

	void PauseScreen::Draw(uint32_t *frame, DS2DEngine::Camera2D *camera, PlayerObj *player, RoomObj *room,
		const std::string &heroName, const char *className)
	{
		gfx_begin(frame);
		Dim(frame);
		const float zoom = camera->Zoom();
		const Vector2 cam = camera->Position();
		auto sx = [&](float wx) { return (wx - cam.X) * zoom + SCREEN_W / 2; };
		auto sy = [&](float wy) { return (wy - cam.Y) * zoom + SCREEN_H / 2; };

		if (room != nullptr)
		{
			Rectangle view = camera->Bounds();
			for (EnemyObj *e : room->EnemyList())
			{
				if (e == nullptr || e->IsKilled || !e->Visible()) continue;
				Rectangle b = e->Bounds();
				if (!b.Intersects(view)) continue;
				Plate(frame, sx(b.X + b.Width / 2.0f), sy((float) b.Top()) - 6, EnemyName(e->Name), {
					{ Text::Loc::Get("LOC_ID_PAUSE_SCREEN_5"), std::to_string(e->Level) },
					{ Text::Loc::Get("LOC_ID_PAUSE_SCREEN_2"), std::to_string((int) e->Damage) },
					{ Text::Loc::Get("LOC_ID_PAUSE_SCREEN_6"), std::to_string((int) e->CurrentHealth) + "/" + std::to_string((int) e->MaxHealth) },
				});
			}
		}
		if (player != nullptr)
		{
			Rectangle b = player->Bounds();
			Plate(frame, sx(b.X + b.Width / 2.0f), sy((float) b.Top()) - 6, heroName, {
				{ Text::Loc::Get("LOC_ID_PAUSE_SCREEN_1"), className },
				{ Text::Loc::Get("LOC_ID_PAUSE_SCREEN_2"), std::to_string((int) player->TotalDamage()) },
			});
		}

		// Title art, centred at the top of the play area (anchor = centre)
		m_title->Position(Vector2(SCREEN_W / 2, 60 + 40));
		m_title->Scale(Vector2(0.75f, 0.75f));
		m_title->Draw(&m_screen);
	}
}
