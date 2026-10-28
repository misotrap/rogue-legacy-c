#include "MapScreen.h"

#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../Text/Loc.h"
#include "../../platform.h"

#include <cstdio>
#include <cstring>
#include <string>

using DS2DEngine::SpriteObj;

namespace RogueCastle
{
	namespace
	{
		const float SCALE_X = 22.0f, SCALE_Y = 22.5f;   // m_spriteScale
		const uint32_t WHITE = 0xFFFFFFFFu, BLACK = 0xFF000000u, GOLD = 0xFFF0C850u;

		// Game.cs zone colours (Color.White, Green, DarkBlue, Red)
		Color AreaColour(Levels::LevelType t)
		{
			switch (t)
			{
			case Levels::LevelType::GARDEN: return Color(0, 128, 0);
			case Levels::LevelType::TOWER: return Color(0, 0, 139);
			case Levels::LevelType::DUNGEON: return Color(255, 0, 0);
			default: return Color(255, 255, 255);
			}
		}

		SpriteObj *Make(const char *name)
		{
			SpriteObj *s = new SpriteObj(name);
			s->ForceDraw(true);
			return s;
		}

		// "[Input:9] pour fermer la carte" -> "SELECT pour fermer la carte"
		std::string WithButton(const char *text, const char *tag, const char *button)
		{
			std::string s = text;
			size_t at = s.find(tag);
			if (at != std::string::npos) s.replace(at, strlen(tag), button);
			return s;
		}
	}

	MapScreen::~MapScreen()
	{
		delete m_door;
		delete m_boss;
		delete m_player;
		for (SpriteObj *s : m_rooms) delete s;
	}

	bool MapScreen::LoadContent(const char *assetsDir)
	{
		bool ok = m_font.Load(assetsDir, "Junicode");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("MapRoom1x1_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "mapSpritesheetBig") && ok;
		m_door = Make("MapDoor_Sprite");
		m_boss = Make("MapBossIcon_Sprite");
		m_boss->AnimationDelay(1 / 30.0f);
		m_boss->PlayAnimation(true);
		m_player = Make("MapPlayerIcon_Sprite");
		m_player->AnimationDelay(1 / 30.0f);
		m_player->PlayAnimation(true);
		m_rooms.assign(100, nullptr);
		m_camera.Position(Vector2(SCREEN_W / 2, SCREEN_H / 2));
		m_camera.Zoom(1.0f);
		return ok;
	}

	SpriteObj *MapScreen::RoomSprite(int w, int h)
	{
		if (w < 1 || h < 1 || w > 9 || h > 9) return nullptr;
		SpriteObj *&s = m_rooms[w * 10 + h];
		if (s == nullptr)
		{
			char name[32];
			snprintf(name, sizeof(name), "MapRoom%dx%d_Sprite", w, h);
			if (!SpriteSystem::SpriteLibrary::ContainsSprite(name)) return nullptr;
			s = Make(name);
		}
		return s;
	}

	void MapScreen::Visit(const Levels::Castle &castle, float x, float y)
	{
		if (m_visited.size() != castle.rooms.size()) Reset(castle.rooms.size());
		for (size_t i = 0; i < castle.rooms.size(); i++)
		{
			const Levels::PlacedRoom &r = castle.rooms[i];
			if (x >= r.x && x < r.Right() && y >= r.y && y < r.Bottom())
			{
				m_visited[i] = true;
				return;
			}
		}
	}

	void MapScreen::Open(float playerX, float playerY)
	{
		m_playerX = playerX;
		m_playerY = playerY;
		m_offsetX = m_offsetY = 0;
	}

	bool MapScreen::Update(uint32_t buttons, uint32_t pressed, double dt)
	{
		m_time += dt;
		const float speed = (float) (300 * dt);   // map pixels per second
		if (buttons & BTN_LEFT) m_offsetX += speed;
		if (buttons & BTN_RIGHT) m_offsetX -= speed;
		if (buttons & BTN_UP) m_offsetY += speed;
		if (buttons & BTN_DOWN) m_offsetY -= speed;
		if (pressed & BTN_A) m_offsetX = m_offsetY = 0;
		return (pressed & (BTN_SELECT | BTN_B)) == 0;
	}

	void MapScreen::Draw(uint32_t *frame, const Levels::Castle &castle)
	{
		gfx_begin(frame);
		// The level screen at 30% under the map
		for (size_t i = 0; i < (size_t) SCREEN_W * SCREEN_H; i++)
		{
			uint32_t c = frame[i];
			frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * 77 >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * 77 >> 8) & 0xFF00u);
		}

		// Map pixel (mx, my) -> screen: the player sits in the middle
		const float ox = SCREEN_W / 2.0f - m_playerX / SCALE_X + m_offsetX;
		const float oy = SCREEN_H / 2.0f - m_playerY / SCALE_Y + m_offsetY;
		m_door->TextureColor(Color(255, 255, 255));

		for (size_t i = 0; i < castle.rooms.size() && i < m_visited.size(); i++)
		{
			if (!m_visited[i]) continue;
			const Levels::PlacedRoom &r = castle.rooms[i];
			SpriteObj *s = RoomSprite(r.width / 1320, r.height / 720);
			if (s == nullptr) continue;   // too large a room is not added
			float sw = s->SpriteRect().Width, sh = s->SpriteRect().Height;
			s->Scale(Vector2((sw - 3) / sw, (sh - 3) / sh));
			s->Position(Vector2(ox + r.x / SCALE_X, oy + r.y / SCALE_Y));
			s->TextureColor(AreaColour(r.levelType));
			s->Draw(&m_camera);

			for (const Levels::Door &d : r.doors)
			{
				if (r.name == "CastleEntrance" && d.pos == Levels::DoorPos::Left) continue;
				float dw = m_door->SpriteRect().Width;
				switch (d.pos)
				{
				case Levels::DoorPos::Left:
					m_door->Rotation(0);
					m_door->Position(Vector2(ox + r.x / SCALE_X - dw + 2, oy + d.y / SCALE_Y - 2));
					break;
				case Levels::DoorPos::Right:
					m_door->Rotation(0);
					m_door->Position(Vector2(ox + r.Right() / SCALE_X - 5, oy + d.y / SCALE_Y - 2));
					break;
				case Levels::DoorPos::Bottom:
					m_door->Rotation(-90);
					m_door->Position(Vector2(ox + d.x / SCALE_X, oy + (d.y + d.height) / SCALE_Y + 2));
					break;
				case Levels::DoorPos::Top:
					m_door->Rotation(-90);
					m_door->Position(Vector2(ox + d.x / SCALE_X, oy + d.y / SCALE_Y + 2));
					break;
				default:
					continue;
				}
				m_door->Draw(&m_camera);
			}

			if (r.name == "EntranceBoss")
			{
				m_boss->Position(Vector2(ox + (r.x + r.width / 2.0f) / SCALE_X - m_boss->Width() / 2 - 1,
					oy + (r.y + r.height / 2.0f) / SCALE_Y - m_boss->Height() / 2 - 2));
				m_boss->Draw(&m_camera);
			}
		}

		// Player icon (the original blinks it)
		if (((int) (m_time * 4)) % 2 == 0)
		{
			m_player->Position(Vector2(ox + m_playerX / SCALE_X - 4, oy + m_playerY / SCALE_Y - 4));
			m_player->Draw(&m_camera);
		}

		// Help texts at the bottom
		std::string close = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_1_NEW"), "[Input:9]", "SELECT");
		std::string centre = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_2_NEW"), "[Input:0]", "A");
		m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_MAP_SCREEN_3"), SCREEN_W / 2, SCREEN_H - 70, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
		m_font.DrawOutlined(frame, centre.c_str(), SCREEN_W / 2, SCREEN_H - 50, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
		m_font.DrawOutlined(frame, close.c_str(), SCREEN_W / 2, SCREEN_H - 30, 0.26f, GOLD, BLACK, 1, Text::Align::Centre);
	}
}
