#include "RoomRenderer.h"
#include "../GameObjects/HoverObj.h"
#include "../../debugflags.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>

#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../SpriteSystem/SpriteLibrary.h"
#include "../../platform.h"

#include <cmath>

using namespace DS2DEngine;

namespace RogueCastle
{
	namespace
	{
		const float PI = 3.14159265f;

		// First image of a sprite: its texture and source rectangle
		bool ImageOf(const std::string &sprite, const Texture *&tex, Rectangle &src)
		{
			tex = SpriteSystem::SpriteLibrary::GetSprite(sprite);
			const std::vector<SpriteSystem::ImageData> *images = SpriteSystem::SpriteLibrary::GetImageDataList(sprite);
			if (tex == nullptr || images == nullptr || images->empty()) return false;
			const SpriteSystem::ImageData &im = (*images)[0];
			src = Rectangle((int) im.SSPos.X, (int) im.SSPos.Y, im.Width, im.Height);
			return im.Width > 0 && im.Height > 0;
		}
	}

	RoomRenderer::~RoomRenderer()
	{
		Clear();
		for (Texture &t : m_clouds) free(t.px);
		free(m_cache);
		free(m_cacheFront);
	}

	void RoomRenderer::Clear()
	{
		m_cacheRoom = nullptr;   // a new castle: the room cache is stale
		for (auto &entry : m_decorations)
		{
			for (GameObj *o : entry.second.back) delete o;
			for (GameObj *o : entry.second.front) delete o;
		}
		m_decorations.clear();
	}

	bool RoomRenderer::LoadContent(const char *assetsDir)
	{
		bool ok = true;
		static const char *const sheets[][2] = { { "CastleFG1_Sprite", "castleTerrainSpritesheet" },
			{ "TowerFG2_Sprite", "towerTerrainSpritesheet" }, { "DungeonFG1_Sprite", "dungeonTerrainSpritesheet" },
			{ "GardenFG_Sprite", "gardenTerrainSpritesheet" } };
		for (const auto &s : sheets)
			if (!SpriteSystem::SpriteLibrary::ContainsSprite(s[0]))
			{
				ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, s[1]) && ok;
				// Magenta marks where the sky shows through (the original's
				// ParallaxFX shader): those texels become transparent, once
				Texture *tex = const_cast<Texture *>(SpriteSystem::SpriteLibrary::GetSprite(s[0]));
				if (tex != nullptr && tex->px != nullptr)
					for (int i = 0; i < gfx_texel_count(tex); i++)
					{
						uint32_t p = tex->px[i];
						if (((p >> 16) & 255) >= 230 && ((p >> 8) & 255) <= 25 && (p & 255) >= 230) tex->px[i] = 0;
					}
			}
		if (!SpriteSystem::SpriteLibrary::ContainsSprite("ParallaxMoon_Sprite"))
			ok = SpriteSystem::SpriteLibrary::LoadSheet(assetsDir, "parallaxBGSpritesheet") && ok;
		return ok;
	}

	// ProceduralLevelScreen: m_backgroundSprite / m_foregroundSprite per
	// level type, and the border textures set in LoadContent
	RoomRenderer::Art RoomRenderer::ArtFor(Levels::LevelType type)
	{
		switch (type)
		{
		case Levels::LevelType::TOWER:
			return { "TowerBG2_Sprite", "TowerFG2_Sprite", "TowerBorder2_Sprite", "TowerCorner_Sprite", "TowerCornerL_Sprite" };
		case Levels::LevelType::DUNGEON:
			return { "DungeonBG1_Sprite", "DungeonFG1_Sprite", "DungeonBorder_Sprite", "DungeonCorner_Sprite", "DungeonCornerL_Sprite" };
		case Levels::LevelType::GARDEN:
			return { "GardenBG_Sprite", "GardenFG_Sprite", "GardenBorder_Sprite", "GardenCorner_Sprite", "GardenCornerL_Sprite" };
		default:
			return { "CastleBG1_Sprite", "CastleFG1_Sprite", "CastleBorder_Sprite", "CastleCorner_Sprite", "CastleCornerL_Sprite" };
		}
	}

	void RoomRenderer::Tile(Camera2D *camera, const std::string &sprite, float x, float y, float w, float h, float scale)
	{
		const Texture *tex;
		Rectangle src;
		if (w <= 0 || h <= 0 || !ImageOf(sprite, tex, src)) return;
		float tw = src.Width * scale, th = src.Height * scale;
		// Tiles line up on a world grid, so neighbouring walls join seamlessly
		float startX = floorf(x / tw) * tw, startY = floorf(y / th) * th;
		Rectangle view = camera->Bounds();
		for (float ty = startY; ty < y + h; ty += th)
		{
			if (ty + th < view.Y || ty > view.Y + view.Height) continue;
			for (float tx = startX; tx < x + w; tx += tw)
			{
				if (tx + tw < view.X || tx > view.X + view.Width) continue;
				// The part of this tile inside the rectangle
				float x0 = fmaxf(tx, x), y0 = fmaxf(ty, y), x1 = fminf(tx + tw, x + w), y1 = fminf(ty + th, y + h);
				int sx = (int) ((x0 - tx) / scale), sy = (int) ((y0 - ty) / scale);
				int sw = (int) ceilf((x1 - x0) / scale), sh = (int) ceilf((y1 - y0) / scale);
				if (sw <= 0 || sh <= 0) continue;
				if (sx + sw > src.Width) sw = src.Width - sx;
				if (sy + sh > src.Height) sh = src.Height - sy;
				camera->Draw(tex, Vector2(x0, y0), Rectangle(src.X + sx, src.Y + sy, sw, sh), Color(255, 255, 255), 0,
					Vector2(0, 0), Vector2(scale, scale), SpriteEffects::None);
			}
		}
	}

	void RoomRenderer::TileRotated(Camera2D *camera, const std::string &sprite, float x, float y, float w, float h, float degrees, float scale)
	{
		const Texture *tex;
		Rectangle src;
		if (w <= 0 || h <= 0 || !ImageOf(sprite, tex, src)) return;
		float rad = degrees * PI / 180.0f, c = cosf(rad), s = sinf(rad);
		// the corners in the world: P0, P0 + w(c, s), + h(-s, c)
		float px[4] = { x, x + w * c, x + w * c - h * s, x - h * s };
		float py[4] = { y, y + w * s, y + w * s + h * c, y + h * c };
		const float zoom = camera->Zoom(), cx = SCREEN_W * 0.5f, cy = 240.0f;   // Camera2D VIEW_CX/CY
		float minY = py[0], maxY = py[0];
		for (int i = 1; i < 4; i++) { minY = fminf(minY, py[i]); maxY = fmaxf(maxY, py[i]); }
		int sy0 = (int) floorf((minY - camera->Y()) * zoom + cy), sy1 = (int) ceilf((maxY - camera->Y()) * zoom + cy);
		if (sy0 < 0) sy0 = 0;      // the whole frame: the room shows behind the HUD bands too
		if (sy1 > SCREEN_H) sy1 = SCREEN_H;
		uint32_t *frame = gfx_frame();
		const float tw = src.Width * scale, th = src.Height * scale;
		for (int sy = sy0; sy < sy1; sy++)
		{
			float wy = (sy + 0.5f - cy) / zoom + camera->Y();
			// where the four edges cross this row
			float xl = 1e9f, xr = -1e9f;
			for (int i = 0; i < 4; i++)
			{
				int j = (i + 1) & 3;
				if ((py[i] <= wy && py[j] > wy) || (py[j] <= wy && py[i] > wy))
				{
					float xi = px[i] + (wy - py[i]) * (px[j] - px[i]) / (py[j] - py[i]);
					xl = fminf(xl, xi);
					xr = fmaxf(xr, xi);
				}
			}
			if (xl > xr) continue;
			int sx0 = (int) ceilf((xl - camera->X()) * zoom + cx - 0.5f), sx1 = (int) floorf((xr - camera->X()) * zoom + cx - 0.5f);
			if (sx0 < 0) sx0 = 0;
			if (sx1 >= SCREEN_W) sx1 = SCREEN_W - 1;
			float fy = fmodf(wy, th);
			if (fy < 0) fy += th;
			const int ty = src.Y + (int) (fy / scale);
			for (int sx = sx0; sx <= sx1; sx++)
			{
				float wx = (sx + 0.5f - cx) / zoom + camera->X();
				float fx = fmodf(wx, tw);
				if (fx < 0) fx += tw;
				uint32_t p = gfx_texel_in(tex, src.X + (int) (fx / scale), ty, src.X, src.Y, src.X + src.Width, src.Y + src.Height);
				if (p >> 24) frame[sy * SCREEN_W + sx] = p;
			}
		}
	}

	void RoomRenderer::Strip(Camera2D *camera, const std::string &sprite, float x, float y, float length, float degrees, float scale)
	{
		const Texture *tex;
		Rectangle src;
		if (length <= 0 || !ImageOf(sprite, tex, src)) return;
		float rad = degrees * PI / 180.0f, dx = cosf(rad), dy = sinf(rad);
		float segment = src.Width * scale;
		for (float done = 0; done < length; done += segment)
		{
			int sw = (int) ceilf(fminf(segment, length - done) / scale);
			if (sw <= 0) break;
			camera->Draw(tex, Vector2(x + dx * done, y + dy * done), Rectangle(src.X, src.Y, sw, src.Height), Color(255, 255, 255), rad,
				Vector2(0, 0), Vector2(scale, scale), SpriteEffects::None);
		}
	}

	// BorderObj.Draw: the border texture runs along each flagged side,
	// leaving a corner's width at both ends; DrawCorners caps the ends
	void RoomRenderer::DrawBorder(Camera2D *camera, const Levels::ObjData &b, const Art &art)
	{
		const Texture *tex;
		Rectangle corner;
		float cw = ImageOf(art.corner, tex, corner) ? corner.Width * 2.0f : 20.0f;
		float offY = art.border.find("Garden") == 0 ? -18.0f : 0.0f; // TextureOffset (garden)
		const float right = b.x + b.width, bottom = b.y + b.height;
		if (b.collidesBottom)
			Strip(camera, art.border, right - cw, bottom - offY, b.width - 2 * cw, 180, 2);
		if (b.collidesLeft)
			Strip(camera, art.border, b.x + offY, bottom - cw, b.height - 2 * cw, -90, 2);
		if (b.collidesRight)
			Strip(camera, art.border, right - offY, b.y + cw, b.height - 2 * cw, 90, 2);
		if (b.collidesTop)
		{
			if (b.rotation == 0)
				Strip(camera, art.border, b.x + cw, b.y + offY, b.width - 2 * cw, 0, 2);
			else
				Strip(camera, art.border, b.x, b.y, b.width, b.rotation, 2); // slopes
		}

		// DrawCorners (simplified: square caps at the ends of each side)
		auto cap = [&](const std::string &sprite, float x, float y, float degrees, bool flip)
		{
			const Texture *t;
			Rectangle src;
			if (!ImageOf(sprite, t, src)) return;
			camera->Draw(t, Vector2(x, y), src, Color(255, 255, 255), degrees * PI / 180.0f, Vector2(0, 0), Vector2(2, 2),
				flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);
		};
		if (b.rotation != 0) return;
		if (b.collidesTop)
		{
			cap(b.collidesRight ? art.cornerL : art.corner, right - (b.collidesRight ? 36 : cw), b.y, 0, false);
			cap(b.collidesLeft ? art.cornerL : art.corner, b.x, b.y, 0, true);
		}
		if (b.collidesBottom)
		{
			cap(b.collidesRight ? art.cornerL : art.corner, right, bottom, 180, true);
			cap(b.collidesLeft ? art.cornerL : art.corner, b.x + (b.collidesLeft ? 36 : cw), bottom, 180, false);
		}
	}

	RoomRenderer::Decorations &RoomRenderer::DecorationsFor(const Levels::PlacedRoom &room)
	{
		auto it = m_decorations.find(&room);
		if (it != m_decorations.end()) return it->second;
		Decorations &d = m_decorations[&room];
		for (const Levels::ObjData &o : room.objs)
		{
			if (o.sprite.empty()) continue;
			bool plain = o.type == "SpriteObj" || o.type == "PhysicsObj";
			// RoomObj builds the containers, except the rotated ones
			bool rotatedContainer = (o.type == "ObjContainer" || o.type == "PhysicsObjContainer") && o.rotation != 0 && !o.breakable;
			if (!plain && !rotatedContainer) continue;
			GameObj *obj;
			std::string sprite = o.sprite;
			// LevelParser.cs line 205: the empty frames get a random painting
			if (sprite == "CastleAssetFrame_Sprite")
				sprite = "FramePicture" + std::to_string(1 + rand() % 16) + "_Sprite";
			// LevelParser.cs line 360: the garden's floating rocks bob forever
			// (Amplitude -50..50, HoverSpeed -2..2). The starting position is
			// stored after placement (ProceduralLevelScreen.cs line 942).
			if (sprite.find("GardenFloatingRock") != std::string::npos && SpriteSystem::SpriteLibrary::ContainsSprite(sprite))
			{
				HoverObj *hover = new HoverObj(sprite,
					-50.0f + 100.0f * rand() / (float) RAND_MAX,   // CDGMath.RandomFloat(-50, 50)
					-2.0f + 4.0f * rand() / (float) RAND_MAX);      // CDGMath.RandomFloat(-2, 2)
				hover->SetStartingPos(Vector2(o.x, o.y));
				if (rl_trace())
					std::printf("hover created %s at (%.0f,%.0f) bg=%d\n", sprite.c_str(), o.x, o.y, (int) o.bgLayer);
				obj = hover;
			}
			else if (SpriteSystem::SpriteLibrary::ContainsCharacter(sprite)) obj = new ObjContainer(sprite);
			else if (SpriteSystem::SpriteLibrary::ContainsSprite(sprite)) obj = new SpriteObj(sprite);
			else continue; // a sheet that is not loaded
			obj->X(o.x);
			obj->Y(o.y);
			obj->Scale(Vector2(o.scaleX, o.scaleY));
			obj->Rotation(o.rotation);
			obj->Flip(o.flip ? SpriteEffects::FlipHorizontally : SpriteEffects::None);
			obj->PlayAnimation(true);
			(o.bgLayer ? d.back : d.front).push_back(obj);
		}
		return d;
	}

	bool RoomRenderer::SkyShowsThrough(Levels::LevelType type)
	{
		auto it = m_skyThrough.find((int) type);
		if (it != m_skyThrough.end()) return it->second;
		bool through = true;
		const Texture *tex;
		Rectangle src;
		if (ImageOf(ArtFor(type).background, tex, src))
		{
			through = false;
			for (int y = src.Y; y < src.Y + src.Height && !through; y += 1 << tex->shift)
				for (int x = src.X; x < src.X + src.Width; x += 1 << tex->shift)
					if ((gfx_texel(tex, x, y) >> 24) != 255) { through = true; break; }
		}
		m_skyThrough[(int) type] = through;
		return through;
	}

	void RoomRenderer::DrawSky(Camera2D *camera)
	{
		// In screen space, like the original's sky render target
		gfx_fill(0, 60, SCREEN_W, 360, 0xFF041D56u); // Color(4, 29, 86)
		Vector2 topLeft(camera->X() - 660, camera->Y() - 360); // Camera.TopLeftCorner
		const Texture *tex;
		Rectangle src;
		if (ImageOf("ParallaxMoon_Sprite", tex, src))
			gfx_blit(tex, src.X, src.Y, src.Width, src.Height, 900 * 0.5f - 10 - topLeft.X * 0.01f * 0.5f,
				200 * 0.5f + 60 - topLeft.Y * 0.01f * 0.5f, 1.0f, 0);
		if (ImageOf("ParallaxDifferenceClouds_Sprite", tex, src))
		{
			// Two layers sliding at 0.2 and 0.4 of the camera, tinted blue:
			// the tinted copies are made once (half size, like the sheets)
			const struct { float speed; uint32_t tint; int flip; float shift; } layers[] = {
				{ 0.2f, 0xFF0A0A50u, 0, 0 }, { 0.4f, 0xFF5050A0u, 1, -250 } };
			int h = src.Height < 360 ? src.Height : 360;
			if (m_clouds[0].px == nullptr)
			{
				m_cloudsW = src.Width;
				m_cloudsH = h;
				for (int i = 0; i < 2; i++)
				{
					Texture &t = m_clouds[i];
					t.w = src.Width;
					t.h = h;
					t.shift = 1;
					t.stride = (src.Width + 1) / 2;
					t.px = (uint32_t *) malloc((size_t) t.stride * ((h + 1) / 2) * 4);
					uint32_t c = layers[i].tint;
					uint32_t cr = (c >> 16) & 255, cg = (c >> 8) & 255, cb = c & 255;
					for (int y = 0; y < (h + 1) / 2; y++)
						for (int x = 0; x < t.stride; x++)
						{
							int fx = src.X + 2 * x + 1, fy = src.Y + 2 * y + 1;
							if (fx >= src.X + src.Width) fx = src.X + src.Width - 1;
							if (fy >= src.Y + h) fy = src.Y + h - 1;
							uint32_t p = gfx_texel(tex, fx, fy);
							uint32_t r = ((p >> 16) & 255) * cr / 255, g = ((p >> 8) & 255) * cg / 255, b = (p & 255) * cb / 255;
							t.px[(size_t) y * t.stride + x] = (p & 0xFF000000u) | (r << 16) | (g << 8) | b;
						}
				}
			}
			for (int i = 0; i < 2; i++)
			{
				const auto &l = layers[i];
				float offset = fmodf(-topLeft.X * l.speed * 0.5f + l.shift, (float) m_cloudsW);
				if (offset > 0) offset -= m_cloudsW;
				for (float x = offset; x < SCREEN_W; x += m_cloudsW)
					gfx_blit(&m_clouds[i], 0, 0, m_cloudsW, m_cloudsH, x, 60, 1.0f, l.flip);
			}
		}
	}

	void RoomRenderer::Draw(Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<GameObj *> *bgObjs)
	{
		const Art art = ArtFor(room.levelType);
		Decorations &d = DecorationsFor(room);
		// 0. the sky is drawn once by the level, before the rooms
		const bool back = m_cachePass != 2, front = m_cachePass != 1;
		// 1. background over the whole room (not in the courtyard: DisplayBG=False)
		if (back && room.name != "Start")
			Tile(camera, art.background, room.x, room.y, (float) room.width, (float) room.height, 2);
		// 2. background-layer decorations
		if (back)
		{
			for (GameObj *o : d.back) if (!m_buildingCache || !Animated(o)) o->Draw(camera);
			if (bgObjs != nullptr)
				for (GameObj *o : *bgObjs) if (!m_buildingCache || !Animated(o)) o->Draw(camera);
		}
		if (!front) return;
		// 3. walls: the foreground tile inside every solid block taller than 40
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "CollHullObj" && o.rotation != 0 && o.height > 40)   // the ramps: no steps showing under them
				TileRotated(camera, art.foreground, o.x, o.y, o.width, o.height, o.rotation, 2);
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "CollHullObj" && o.rotation == 0 && o.height > 40
			    && o.name != "bridge" && o.name.find("Block") == std::string::npos)   // ShowTerrain / the stands' roofs
				Tile(camera, art.foreground, o.x, o.y, o.width, o.height, 2);
		// 4. borders
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "BorderObj") DrawBorder(camera, o, art);
		// the spikes (HazardObj.Draw: the sprite repeated along the width)
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "HazardObj") Strip(camera, "Spikes_Sprite", o.x, o.y, o.width, o.rotation, 1);
		// 5. the other decorations
		for (GameObj *o : d.front) if (!m_buildingCache || !Animated(o)) o->Draw(camera);
	}

	void RoomRenderer::BuildCache(Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<GameObj *> *bgObjs)
	{
		// The room at the screen's scale (zoom 0.5), drawn screen by screen
		const float zoom = camera->Zoom();
		int w = (int) (room.width * zoom), h = (int) (room.height * zoom);
		if ((size_t) w * h > m_cacheCapacity)
		{
			free(m_cache);
			m_cacheCapacity = (size_t) w * h;
			m_cache = (uint32_t *) malloc(m_cacheCapacity * 4);
			if (m_cache == nullptr) { m_cacheCapacity = 0; m_cacheRoom = nullptr; return; }
		}
		m_cacheW = w;
		m_cacheH = h;
		free(m_cacheFront);
		m_cacheFront = (uint32_t *) malloc((size_t) w * h * 4);
		if (m_cacheFront == nullptr) { m_cacheRoom = nullptr; return; }
		static std::vector<uint32_t> screen;
		screen.assign((size_t) SCREEN_W * SCREEN_H, 0);
		const Vector2 keep = camera->Position();
		camera->ViewHeight(SCREEN_H);
		m_buildingCache = true;
		for (int pass = 1; pass <= 2; pass++)
		{
			m_cachePass = pass;
			uint32_t *layer = pass == 1 ? m_cache : m_cacheFront;
			for (int cy = 0; cy < h; cy += SCREEN_H)
				for (int cx = 0; cx < w; cx += SCREEN_W)
				{
					// The camera that puts this piece of the room at the screen's top-left
					camera->Position(Vector2(room.x + (cx + SCREEN_W * 0.5f) / zoom, room.y + (cy + 240) / zoom));
					std::fill(screen.begin(), screen.end(), 0u);
					gfx_begin(screen.data());
					Draw(camera, room, bgObjs);
					for (int y = 0; y < SCREEN_H && cy + y < h; y++)
					{
						int n = w - cx < SCREEN_W ? w - cx : SCREEN_W;
						memcpy(layer + (size_t) (cy + y) * w + cx, screen.data() + (size_t) y * SCREEN_W, (size_t) n * 4);
					}
				}
		}
		m_cachePass = 0;
		// The front layer's visible span on each row
		m_frontSpan.assign((size_t) h * 2, 0);
		for (int y = 0; y < h; y++)
		{
			const uint32_t *row = m_cacheFront + (size_t) y * w;
			int a = 0, b = w;
			while (a < w && (row[a] >> 24) == 0) a++;
			while (b > a && (row[b - 1] >> 24) == 0) b--;
			m_frontSpan[2 * y] = a;
			m_frontSpan[2 * y + 1] = b;
		}
		m_buildingCache = false;
		camera->ViewHeight(360);
		camera->Position(keep);
		m_cacheRoom = &room;
		m_cacheOpaque = true;
		size_t holes = 0, partial = 0;
		for (size_t i = 0; i < (size_t) w * h; i++)
		{
			uint32_t a = m_cache[i] >> 24;
			if (a == 0) holes++;
			else if (a != 255) partial++;
		}
		m_cacheOpaque = holes == 0;   // the screen ignores alpha: no hole, no sky behind
		if (rl_trace())
			fprintf(stderr, "room cache %s: %dx%d, %zu holes, %zu partial\n", room.name.c_str(), w, h, holes, partial);
		if (const char *dump = getenv("RL_DUMP_CACHE"))
			if (FILE *out = fopen(dump, "wb"))
			{
				fprintf(out, "P6\n%d %d\n255\n", w, h);
				for (size_t i = 0; i < (size_t) w * h; i++)
				{
					unsigned char rgb[3] = { (unsigned char) (m_cache[i] >> 16), (unsigned char) (m_cache[i] >> 8), (unsigned char) m_cache[i] };
					fwrite(rgb, 1, 3, out);
				}
				fclose(out);
			}
	}

	void RoomRenderer::DrawCached(Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<GameObj *> *bgObjs)
	{
		// Screen (sx, sy) shows the cache's (sx + ox, sy + oy)
		const float zoom = camera->Zoom();
		int ox = (int) lroundf((camera->X() - room.x) * zoom - SCREEN_W * 0.5f);
		int oy = (int) lroundf((camera->Y() - room.y) * zoom - 240);
		uint32_t *frame = gfx_frame();
		int x0 = ox < 0 ? -ox : 0, x1 = m_cacheW - ox < SCREEN_W ? m_cacheW - ox : SCREEN_W;
		for (int sy = 0; sy < SCREEN_H && x0 < x1; sy++)   // the room fills the HUD bands too
		{
			int cy = sy + oy;
			if (cy < 0 || cy >= m_cacheH) continue;
			const uint32_t *src = m_cache + (size_t) cy * m_cacheW + ox;
			uint32_t *dst = frame + (size_t) sy * SCREEN_W;
			if (m_cacheOpaque)
				memcpy(dst + x0, src + x0, (size_t) (x1 - x0) * 4);
			else
				for (int x = x0; x < x1; x++)
				{
					uint32_t p = src[x], a = p >> 24;
					if (a == 255) dst[x] = p;
					else if (a != 0)
					{
						uint32_t d = dst[x], ia = 255 - a;
						uint32_t rb = ((d & 0x00FF00FFu) * ia + 0x00800080u) >> 8 & 0x00FF00FFu;
						uint32_t g = ((d & 0x0000FF00u) * ia + 0x00008000u) >> 8 & 0x0000FF00u;
						dst[x] = p + rb + g;
					}
				}
		}
		gfx_pixels += (uint64_t) (x1 > x0 ? x1 - x0 : 0) * SCREEN_H;
		// The animated background decorations, then the front layer over them
		Decorations &d = DecorationsFor(room);
		for (GameObj *o : d.back) if (Animated(o)) o->Draw(camera);
		if (bgObjs != nullptr)
			for (GameObj *o : *bgObjs) if (Animated(o)) o->Draw(camera);
		for (int sy = 0; sy < SCREEN_H && x0 < x1; sy++)
		{
			int cy = sy + oy;
			if (cy < 0 || cy >= m_cacheH) continue;
			int a = m_frontSpan[2 * cy] - ox, b = m_frontSpan[2 * cy + 1] - ox;
			if (a < x0) a = x0;
			if (b > x1) b = x1;
			const uint32_t *src = m_cacheFront + (size_t) cy * m_cacheW + ox;
			uint32_t *dst = frame + (size_t) sy * SCREEN_W;
			for (int x = a; x < b; x++)
			{
				uint32_t p = src[x], al = p >> 24;
				if (al == 0) continue;
				if (al == 255) { dst[x] = p; continue; }
				uint32_t dd = dst[x], ia = 255 - al;
				uint32_t rb = ((dd & 0x00FF00FFu) * ia + 0x00800080u) >> 8 & 0x00FF00FFu;
				uint32_t g = (((dd >> 8) & 0x00FF00FFu) * ia + 0x00800080u) & 0xFF00FF00u;
				dst[x] = p + rb + g;
			}
		}
		// The animated front decorations
		for (GameObj *o : d.front) if (Animated(o)) o->Draw(camera);
	}
}
