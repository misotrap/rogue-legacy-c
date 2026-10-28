#include "RoomRenderer.h"

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
	}

	void RoomRenderer::Clear()
	{
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
					for (int i = 0; i < tex->w * tex->h; i++)
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
			if (SpriteSystem::SpriteLibrary::ContainsCharacter(o.sprite)) obj = new ObjContainer(o.sprite);
			else if (SpriteSystem::SpriteLibrary::ContainsSprite(o.sprite)) obj = new SpriteObj(o.sprite);
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
			// Two layers sliding at 0.2 and 0.4 of the camera, tinted blue
			const struct { float speed; uint32_t tint; int flip; float shift; } layers[] = {
				{ 0.2f, 0xFF0A0A50u, 0, 0 }, { 0.4f, 0xFF5050A0u, 1, -250 } };
			for (const auto &l : layers)
			{
				float offset = fmodf(-topLeft.X * l.speed * 0.5f + l.shift, (float) src.Width);
				if (offset > 0) offset -= src.Width;
				for (float x = offset; x < SCREEN_W; x += src.Width)
					gfx_blit_tinted(tex, src.X, src.Y, src.Width, src.Height < 360 ? src.Height : 360, x, 60, 1.0f, l.flip, l.tint);
			}
		}
	}

	void RoomRenderer::Draw(Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<GameObj *> *bgObjs)
	{
		const Art art = ArtFor(room.levelType);
		Decorations &d = DecorationsFor(room);
		// 0. sky (only seen through the transparent parts of the art)
		DrawSky(camera);
		// 1. background over the whole room (not in the courtyard: DisplayBG=False)
		if (room.name != "Start")
			Tile(camera, art.background, room.x, room.y, (float) room.width, (float) room.height, 2);
		// 2. background-layer decorations
		for (GameObj *o : d.back) o->Draw(camera);
		if (bgObjs != nullptr)
			for (GameObj *o : *bgObjs) o->Draw(camera);
		// 3. walls: the foreground tile inside every solid block taller than 40
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "CollHullObj" && o.rotation == 0 && o.height > 40)
				Tile(camera, art.foreground, o.x, o.y, o.width, o.height, 2);
		// 4. borders
		for (const Levels::ObjData &o : room.objs)
			if (o.type == "BorderObj") DrawBorder(camera, o, art);
		// 5. the other decorations
		for (GameObj *o : d.front) o->Draw(camera);
	}
}
