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
		delete m_chestLocked;
		delete m_chestFairy;
		delete m_chestOpened;
		delete m_bonus;
		delete m_teleporter;
		delete m_legendPlate;
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
		// MapObj.AddRoom: the chest, bonus and teleporter icons
		m_chestLocked = Make("MapLockedChestIcon_Sprite");
		m_chestLocked->AnimationDelay(1 / 30.0f);
		m_chestLocked->PlayAnimation(true);
		m_chestFairy = Make("MapFairyChestIcon_Sprite");
		m_chestFairy->AnimationDelay(1 / 30.0f);
		m_chestFairy->PlayAnimation(true);
		m_chestOpened = Make("MapChestUnlocked_Sprite");
		m_bonus = Make("MapBonusIcon_Sprite");
		m_bonus->AnimationDelay(1 / 30.0f);
		m_bonus->PlayAnimation(true);
		m_teleporter = Make("MapTeleporterIcon_Sprite");
		m_teleporter->AnimationDelay(1 / 30.0f);
		m_teleporter->PlayAnimation(true);
		if (SpriteSystem::SpriteLibrary::ContainsSprite("TraitsScreenPlate_Sprite"))
			m_legendPlate = Make("TraitsScreenPlate_Sprite");   // MapScreen.cs: the legend background
		else if (SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "miscSpritesheet"))
			m_legendPlate = Make("TraitsScreenPlate_Sprite");
		/* TeleporterTitleText_Sprite: the localized bitmap of the
		 * selection screen (Game.ChangeBitmapLanguage); the port speaks
		 * French, like Loc::Load("fr"). */
		const char *title = "TeleporterTitleText_Sprite_FR";
		if (!SpriteSystem::SpriteLibrary::ContainsSprite(title))
			SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "language2Spritesheet");
		if (!SpriteSystem::SpriteLibrary::ContainsSprite(title))
			title = "TeleporterTitleText_Sprite";
		if (SpriteSystem::SpriteLibrary::ContainsSprite(title))
			m_titleText = Make(title);
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
		m_isTeleporter = false;
	}

	void MapScreen::OpenTeleporter(float playerX, float playerY, const Levels::Castle &castle)
	{
		Open(playerX, playerY);
		m_isTeleporter = true;
		m_selectedTeleporter = 0;
		m_teleportChoice = -1;
		m_dests.clear();
		/* The icons the C# adds as the rooms are visited (MapObj.AddRoom);
		 * the hero always visits the CastleEntrance first, so its
		 * teleporter is index 0 (MapObj.TeleportPlayer's hack). The
		 * spelunker has every room from the start. */
		for (size_t pass = 0; pass < 2; pass++)
			for (size_t i = 0; i < castle.rooms.size(); i++)
			{
				const Levels::PlacedRoom &r = castle.rooms[i];
				const char *n = r.name.c_str();
				if (strcmp(n, "Linker") != 0 && strcmp(n, "EntranceBoss") != 0 && strcmp(n, "CastleEntrance") != 0)
					continue;
				bool entrance = strcmp(n, "CastleEntrance") == 0;
				if (pass == 0 ? !entrance : entrance)
					continue;
				if (i < m_visited.size() && (m_visited[i] || m_spelunker))
					m_dests.push_back((int) i);
			}
		if (m_selectedTeleporter >= (int) m_dests.size())
			m_selectedTeleporter = 0;
	}

	bool MapScreen::TeleportConfirmed(int &roomIndex)
	{
		if (m_teleportChoice < 0)
			return false;
		roomIndex = m_dests[m_teleportChoice];
		m_teleportChoice = -1;
		return true;
	}

	bool MapScreen::Update(uint32_t buttons, uint32_t pressed, double dt)
	{
		m_time += dt;
		if (m_isTeleporter && !m_dests.empty())
		{
			/* MapScreen.cs line 320: Left/Right cycle the destinations,
			 * A travels, the map buttons close. */
			if (pressed & BTN_LEFT)
				m_selectedTeleporter = (m_selectedTeleporter + (int) m_dests.size() - 1) % (int) m_dests.size();
			if (pressed & BTN_RIGHT)
				m_selectedTeleporter = (m_selectedTeleporter + 1) % (int) m_dests.size();
			if ((pressed & BTN_A) != 0)
			{
				m_teleportChoice = m_selectedTeleporter;
				return false;
			}
		}
		else
		{
			const float speed = (float) (300 * dt);   // map pixels per second
			if (buttons & BTN_LEFT) m_offsetX += speed;
			if (buttons & BTN_RIGHT) m_offsetX -= speed;
			if (buttons & BTN_UP) m_offsetY += speed;
			if (buttons & BTN_DOWN) m_offsetY -= speed;
			if (pressed & BTN_A) m_offsetX = m_offsetY = 0;
		}
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

		// Map pixel (mx, my) -> screen: the player sits in the middle.
		// The teleporter screen draws the visited rooms too (MapObj always
		// draws them; only the icon list is teleporter-only), centred on
		// the selected destination.
		if (m_isTeleporter && !m_dests.empty() && m_selectedTeleporter < (int) m_dests.size())
		{
			const Levels::PlacedRoom &sel = castle.rooms[m_dests[m_selectedTeleporter]];
			DrawRooms(castle, SCREEN_W * 0.5f - (sel.x + sel.width / 2.0f) / SCALE_X,
				SCREEN_H * 0.5f - (sel.y + sel.height / 2.0f) / SCALE_Y, 1);
		}
		else if (alzheimers)
			m_font.DrawOutlined(frame, "?????", SCREEN_W / 2, SCREEN_H / 2 - 20, 0.8f, WHITE, BLACK, 2, Text::Align::Centre);
		else
			DrawRooms(castle, SCREEN_W / 2.0f - m_playerX / SCALE_X + m_offsetX,
				SCREEN_H / 2.0f - m_playerY / SCALE_Y + m_offsetY, 1);

		// Help texts at the bottom
		std::string close = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_1_NEW"), "[Input:9]", "SELECT");
		if (m_isTeleporter)
		{
			/* The selection screen: the visited rooms behind, the
			 * teleporter icons centred on the selected one, the title and
			 * the player marker (MapObj DrawTeleportersOnly). */
			DrawTeleporters(castle);
			m_font.DrawOutlined(frame, "Gauche/Droite : choisir, A : voyager",
				SCREEN_W / 2, SCREEN_H - 50, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, close.c_str(), SCREEN_W / 2, SCREEN_H - 30, 0.26f, GOLD, BLACK, 1, Text::Align::Centre);
		}
		else
		{
			std::string centre = WithButton(Text::Loc::Get("LOC_ID_MAP_SCREEN_2_NEW"), "[Input:0]", "A");
			m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_MAP_SCREEN_3"), SCREEN_W / 2, SCREEN_H - 70, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, centre.c_str(), SCREEN_W / 2, SCREEN_H - 50, 0.26f, WHITE, BLACK, 1, Text::Align::Centre);
			m_font.DrawOutlined(frame, close.c_str(), SCREEN_W / 2, SCREEN_H - 30, 0.26f, GOLD, BLACK, 1, Text::Align::Centre);
		}

		// The legend (MapScreen.cs LoadContent): a plate at the bottom
		// right with an icon and a text per row. The original is twice this
		// size on a 1320x720 screen.
		if (m_legendPlate != nullptr)
		{
			const float pw = m_legendPlate->SpriteRect().Width * 0.375f, ph = m_legendPlate->SpriteRect().Height * 0.29f;
			const float px = SCREEN_W - pw - 10, py = SCREEN_H - ph - 10;
			m_legendPlate->Position(Vector2(px, py));
			m_legendPlate->Scale(Vector2(0.375f, 0.29f));
			m_legendPlate->Draw(&m_camera);
			const uint32_t TITLE = 0xFFD5D5ADu;   // Color(213, 213, 173)
			m_font.DrawOutlined(frame, Text::Loc::Get("LOC_ID_MAP_SCREEN_5"), (int) (px + pw / 2), (int) (py + 10),
				0.13f, TITLE, BLACK, 1, Text::Align::Centre);
			static const char *const ids[] = { "LOC_ID_MAP_SCREEN_6", "LOC_ID_MAP_SCREEN_7", "LOC_ID_MAP_SCREEN_8",
				"LOC_ID_MAP_SCREEN_9", "LOC_ID_MAP_SCREEN_10", "LOC_ID_MAP_SCREEN_11", "LOC_ID_MAP_SCREEN_12" };
			SpriteObj *const icons[] = { m_player, m_boss, m_chestLocked, m_chestFairy, m_chestOpened, m_teleporter, m_bonus };
			const float step = 13.0f;
			for (int i = 0; i < 7; i++)
			{
				if (icons[i] == nullptr) continue;
				float iy = py + 26 + step * i;
				Vector2 keep = icons[i]->Position(), keepScale = icons[i]->Scale();
				icons[i]->Scale(Vector2(0.5f, 0.5f));   // the 20x20 map icons, legend size
				icons[i]->Position(Vector2(px + 14, iy));
				icons[i]->Opacity(1);
				icons[i]->Draw(&m_camera);
				icons[i]->Position(keep);
				icons[i]->Scale(keepScale);
				m_font.DrawOutlined(frame, Text::Loc::Get(ids[i]), (int) (px + 34), (int) (iy - 4),
					0.13f, WHITE, BLACK, 1, Text::Align::Left);
			}
		}
	}
	/* The teleporter selection's icons (MapObj.DrawRenderTargets draws
	 * only the teleporter icons in this mode), the screen centred on the
	 * selected one (CentreAroundTeleporter), the title at the top and the
	 * player marker on it (the C# draws a small red walking knight). The
	 * visited rooms are drawn by Draw(). */
	void MapScreen::DrawTeleporters(const Levels::Castle &castle)
	{
		if (m_dests.empty() || m_selectedTeleporter >= (int) m_dests.size())
			return;
		const float w = m_teleporter->SpriteRect().Width, h = m_teleporter->SpriteRect().Height;
		auto iconPos = [&](int roomIndex)
		{
			const Levels::PlacedRoom &r = castle.rooms[roomIndex];
			return Vector2((r.x + r.width / 2.0f) / SCALE_X - w / 2.0f - 1,
				(r.y + r.height / 2.0f) / SCALE_Y - h / 2.0f - 2);
		};
		Vector2 sel = iconPos(m_dests[m_selectedTeleporter]);
		float ox = SCREEN_W * 0.5f - sel.X, oy = SCREEN_H * 0.5f - sel.Y;
		m_teleporter->Scale(Vector2(1, 1));
		for (int roomIndex : m_dests)
		{
			Vector2 p = iconPos(roomIndex);
			m_teleporter->Position(Vector2(ox + p.X, oy + p.Y));
			m_teleporter->Draw(&m_camera);
		}
		m_player->Scale(Vector2(1, 1));
		m_player->Position(Vector2(ox + sel.X + 7, oy + sel.Y - 10));
		m_player->Draw(&m_camera);
		if (m_titleText != nullptr)
		{
			m_titleText->Scale(Vector2(0.5f, 0.5f));
			m_titleText->Position(Vector2(SCREEN_W * 0.5f, SCREEN_H * 0.1f));
			m_titleText->Draw(&m_camera);
		}
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
		m_chestLocked->Scale(Vector2(k, k));
		m_chestFairy->Scale(Vector2(k, k));
		m_chestOpened->Scale(Vector2(k, k));
		m_bonus->Scale(Vector2(k, k));
		m_teleporter->Scale(Vector2(k, k));
		// Room centre in map pixels, the boss / bonus / teleporter icon
		// position formula of MapObj.AddRoom
		auto centreIcon = [k](float rx, float ry, float w, float h)
		{
			return Vector2((rx / SCALE_X - w / 2.0f - 1) * k, (ry / SCALE_Y - h / 2.0f - 2) * k);
		};
		for (size_t i = 0; i < castle.rooms.size() && i < m_visited.size(); i++)
		{
			if (!m_visited[i] && !m_spelunker) continue;
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
				m_boss->Position(centreIcon(r.x + r.width / 2.0f, r.y + r.height / 2.0f,
					m_boss->SpriteRect().Width, m_boss->SpriteRect().Height) + Vector2(ox, oy));
				m_boss->Draw(&m_camera);
			}
			// MapObj.AddRoom: the bonus room icon (skipped for the
			// spelunker, who sees every room anyway)
			if (r.name == "Bonus")
			{
				m_bonus->Position(centreIcon(r.x + r.width / 2.0f, r.y + r.height / 2.0f,
					m_bonus->SpriteRect().Width, m_bonus->SpriteRect().Height) + Vector2(ox, oy));
				m_bonus->Draw(&m_camera);
			}
			// The teleporter rooms (the icons double as the destinations of
			// the teleporter screen)
			if (r.name == "Linker" || r.name == "EntranceBoss" || r.name == "CastleEntrance")
			{
				m_teleporter->Position(centreIcon(r.x + r.width / 2.0f, r.y + r.height / 2.0f,
					m_teleporter->SpriteRect().Width, m_teleporter->SpriteRect().Height) + Vector2(ox, oy));
				m_teleporter->Draw(&m_camera);
			}
			// The chests of the room (MapObj.AddRoom skips the bonus rooms,
			// which have many, and everything for the spelunker). The icon
			// sits at the chest itself.
			if (r.name != "Bonus" && !m_spelunker)
			{
				for (const MapChest &c : m_chests)
				{
					if (c.x < r.x || c.x >= r.Right() || c.y < r.y || c.y >= r.Bottom()) continue;
					SpriteObj *icon = c.kind == MapChest::OPENED ? m_chestOpened
						: (c.kind == MapChest::FAIRY || c.kind == MapChest::FAIRY_INVISIBLE) ? m_chestFairy
						: m_chestLocked;
					icon->Opacity(c.kind == MapChest::FAIRY_INVISIBLE ? 0.2f : 1.0f);
					icon->Position(Vector2(ox + (c.x / SCALE_X - 8) * k, oy + (c.y / SCALE_Y - 12) * k));
					icon->Draw(&m_camera);
				}
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
