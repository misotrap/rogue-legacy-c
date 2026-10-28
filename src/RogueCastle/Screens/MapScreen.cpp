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
		delete m_miniBG;
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
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("MinimapBG_Sprite"))
			SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "uiExtraSpritesheet");
		if (SpriteSystem::SpriteLibrary::ContainsSprite("MinimapBG_Sprite")) m_miniBG = Make("MinimapBG_Sprite");
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

	void MapScreen::Draw(uint32_t *frame, const Levels::Castle &castle, bool alzheimers)
	{
		gfx_begin(frame);
		// The level screen at 30% under the map
		for (size_t i = 0; i < (size_t) SCREEN_W * SCREEN_H; i++)
		{
			uint32_t c = frame[i];
			frame[i] = 0xFF000000u | (((c & 0xFF00FFu) * 77 >> 8) & 0xFF00FFu) | (((c & 0xFF00u) * 77 >> 8) & 0xFF00u);
		}

		// Map pixel (mx, my) -> screen: the player sits in the middle
		if (alzheimers)
			m_font.DrawOutlined(frame, "?????", SCREEN_W / 2, SCREEN_H / 2 - 20, 0.8f, WHITE, BLACK, 2, Text::Align::Centre);
		else
			DrawRooms(castle, SCREEN_W / 2.0f - m_playerX / SCALE_X + m_offsetX,
				SCREEN_H / 2.0f - m_playerY / SCALE_Y + m_offsetY, 1);

		// Help texts at the bottom
		std::string close = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_1_NEW"), "[Input:9]", "SELECT");
		std::string centre = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_2_NEW"), "[Input:0]", "A");
		m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_MAP_SCREEN_3"), SCREEN_W / 2, SCREEN_H - 70, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
		m_font.DrawOutlined(frame, centre.c_str(), SCREEN_W / 2, SCREEN_H - 50, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
		m_font.DrawOutlined(frame, close.c_str(), SCREEN_W / 2, SCREEN_H - 30, 0.26f, GOLD, BLACK, 1, Text::Align::Centre);
	}
	// MapObj.Draw: the visited rooms in their area colours, their doors, the
	// boss icon and the blinking player icon. Map pixel p -> screen o + p * k
	// (k = 1 for the map screen, 0.5 for the minimap).
	void MapScreen::DrawRooms(const Levels::Castle &castle, float ox, float oy, float k)
	{
		m_door->TextureColor(Color(255, 255, 255));
		m_door->Scale(Vector2(k, k));
		m_boss->Scale(Vector2(k, k));
		m_player->Scale(Vector2(k, k));
		for (size_t i = 0; i < castle.rooms.size() && i < m_visited.size(); i++)
		{
			if (!m_visited[i]) continue;
			const Levels::PlacedRoom &r = castle.rooms[i];
			SpriteObj *s = RoomSprite(r.width / 1320, r.height / 720);
			if (s == nullptr) continue;   // too large a room is not added
			float sw = s->SpriteRect().Width, sh = s->SpriteRect().Height;
			s->Scale(Vector2((sw - 3) / sw * k, (sh - 3) / sh * k));
			s->Position(Vector2(ox + r.x / SCALE_X * k, oy + r.y / SCALE_Y * k));
			s->TextureColor(AreaColour(r.levelType));
			s->Draw(&m_camera);

			for (const Levels::Door &d : r.doors)
			{
				if (r.name == "CastleEntrance" && d.pos == Levels::DoorPos::Left) continue;
				float dw = m_door->SpriteRect().Width;
				float mx, my;   // map pixels
				switch (d.pos)
				{
				case Levels::DoorPos::Left: m_door->Rotation(0); mx = r.x / SCALE_X - dw + 2; my = d.y / SCALE_Y - 2; break;
				case Levels::DoorPos::Right: m_door->Rotation(0); mx = r.Right() / SCALE_X - 5; my = d.y / SCALE_Y - 2; break;
				case Levels::DoorPos::Bottom: m_door->Rotation(-90); mx = d.x / SCALE_X; my = (d.y + d.height) / SCALE_Y + 2; break;
				case Levels::DoorPos::Top: m_door->Rotation(-90); mx = d.x / SCALE_X; my = d.y / SCALE_Y + 2; break;
				default: continue;
				}
				m_door->Position(Vector2(ox + mx * k, oy + my * k));
				m_door->Draw(&m_camera);
			}

			if (r.name == "EntranceBoss")
			{
				m_boss->Position(Vector2(ox + ((r.x + r.width / 2.0f) / SCALE_X - m_boss->SpriteRect().Width / 2.0f - 1) * k,
					oy + ((r.y + r.height / 2.0f) / SCALE_Y - m_boss->SpriteRect().Height / 2.0f - 2) * k));
				m_boss->Draw(&m_camera);
			}
		}

		// Player icon (the original blinks it)
		if (((int) (m_time * 4)) % 2 == 0)
		{
			m_player->Position(Vector2(ox + (m_playerX / SCALE_X - 4) * k, oy + (m_playerY / SCALE_Y - 4) * k));
			m_player->Draw(&m_camera);
		}
	}

	// The minimap (ProceduralLevelScreen: MapObj in Rectangle(1320 - 250,
	// 50, 200, 100) over MinimapBG): half size on the 640x480 screen, the
	// player in the middle, clipped to the frame through a scratch buffer.
	void MapScreen::DrawMini(uint32_t *frame, const Levels::Castle &castle, float playerX, float playerY, double dt)
	{
		const int X = (1320 - 250 - 660) / 2 + SCREEN_W / 2, Y = (50 - 360) / 2 + SCREEN_H / 2, W = 100, H = 50;
		const uint32_t KEY = 0xFF010203u;
		m_time += dt;
		m_playerX = playerX;
		m_playerY = playerY;
		gfx_begin(frame);
		if (m_miniBG != nullptr)
		{
			m_miniBG->Position(Vector2((float) X, (float) Y));
			m_miniBG->Scale(Vector2(0.5f, 0.5f));
			m_miniBG->Draw(&m_camera);
		}
		m_scratch.resize((size_t) SCREEN_W * SCREEN_H);
		for (int y = Y; y < Y + H; y++)
			for (int x = X; x < X + W; x++) m_scratch[(size_t) y * SCREEN_W + x] = KEY;
		gfx_begin(m_scratch.data());
		DrawRooms(castle, X + W / 2.0f - playerX / SCALE_X * 0.5f, Y + H / 2.0f - playerY / SCALE_Y * 0.5f, 0.5f);
		gfx_begin(frame);
		for (int y = Y; y < Y + H; y++)
			for (int x = X; x < X + W; x++)
			{
				uint32_t c = m_scratch[(size_t) y * SCREEN_W + x];
				if (c != KEY) frame[(size_t) y * SCREEN_W + x] = c;
			}
	}
}
