// The original look of a room, drawn behind the game objects:
//   0. the night sky (SkyObj), behind the magenta windows of the art,
//   1. the area background (m_backgroundSprite, tiled),
//   2. decorations on the background layer (BGLayer),
//   3. the walls wallpapered with the area's foreground tile (the BWMask
//      render target of ProceduralLevelScreen: CollHullObj taller than 40),
//   4. the wall borders and their corners (BorderObj.Draw / DrawCorners),
//   5. the other plain sprite decorations (SpriteObj, PhysicsObj), and the
//      rotated containers the room objects skip.
// RoomObj then draws its containers, breakables and enemies on top.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/GameObj.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../Levels/RoomData.h"

#include <map>
#include <string>
#include <vector>

namespace RogueCastle
{
	class RoomRenderer
	{
	public:
		~RoomRenderer();
		// Loads the four area sheets (castle, tower, dungeon, garden terrain)
		bool LoadContent(const char *assetsDir);
		// Everything behind the room objects (steps 1 to 5 above)
		// bgObjs: the room's Layer -1 containers, drawn behind the walls
		void Draw(DS2DEngine::Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<DS2DEngine::GameObj *> *bgObjs = nullptr);
		// Frees the decorations built for the rooms (on a new castle)
		void Clear();
		// SkyObj: night sky, moon and two cloud layers, seen through the
		// transparent parts of the backgrounds (ParallaxFX in the original).
		// Drawn once per frame, before the rooms, and only when it shows.
		void DrawSky(DS2DEngine::Camera2D *camera);
		// The room's still art (background, walls, borders, spikes, still
		// decorations), drawn once into a room-sized image when the hero
		// enters it; each frame then copies the visible part and draws the
		// animated decorations on top. The sky shows through its holes.
		void BuildCache(DS2DEngine::Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<DS2DEngine::GameObj *> *bgObjs);
		void DrawCached(DS2DEngine::Camera2D *camera, const Levels::PlacedRoom &room, const std::vector<DS2DEngine::GameObj *> *bgObjs);
		bool CacheIsFor(const Levels::PlacedRoom &room) const { return m_cacheRoom == &room && m_cache != nullptr; }
		bool CacheOpaque() const { return m_cacheOpaque; }
		// Whether the sky shows through this area's background (its image
		// has transparent texels; looked up once)
		bool SkyShowsThrough(Levels::LevelType type);

	private:
		struct Art
		{
			std::string background, foreground, border, corner, cornerL;
		};
		struct Decorations
		{
			std::vector<DS2DEngine::GameObj *> back, front;
		};
		std::map<const Levels::PlacedRoom *, Decorations> m_decorations;
		/* The ancestor shown in each portrait room, chosen by the level
		 * (one random per castle, like PortraitRoomObj.OnEnter). */
		std::map<const Levels::PlacedRoom *, int> m_portraitIndex;
		/* The vita chambers' fountains, to grey them out once used
		 * (VitaChamberRoomObj.OnEnter). */
		std::map<const Levels::PlacedRoom *, DS2DEngine::SpriteObj *> m_fountains;
		std::map<const Levels::PlacedRoom *, uint32_t> m_fountainTint;
	public:
		void SetPortraitIndex(const Levels::PlacedRoom &room, int index) { m_portraitIndex[&room] = index; }
		void SetFountainTint(const Levels::PlacedRoom &room, uint32_t colour)
		{
			m_fountainTint[&room] = colour;
			auto it = m_fountains.find(&room);
			if (it != m_fountains.end() && it->second != nullptr)
				it->second->TextureColor(Color(colour >> 16 & 255, colour >> 8 & 255, colour & 255));
		}
	private:

		static Art ArtFor(Levels::LevelType type);
		Decorations &DecorationsFor(const Levels::PlacedRoom &room);
		// Draws a sprite's first image repeated over a world rectangle
		static void Tile(DS2DEngine::Camera2D *camera, const std::string &sprite, float x, float y, float w, float h, float scale);
		// The same tiling inside a rectangle turned around its top-left
		// corner (the 45 degree ramps), scanline by scanline
		static void TileRotated(DS2DEngine::Camera2D *camera, const std::string &sprite, float x, float y, float w, float h, float degrees, float scale);
		// Draws a sprite's first image repeated along a line of 'length'
		// world units starting at (x, y), turned by 'degrees'
		static void Strip(DS2DEngine::Camera2D *camera, const std::string &sprite, float x, float y, float length, float degrees, float scale);
		static void DrawBorder(DS2DEngine::Camera2D *camera, const Levels::ObjData &b, const Art &art);
		std::map<int, bool> m_skyThrough;
		uint32_t *m_cache = nullptr;        // the background layer
		uint32_t *m_cacheFront = nullptr;   // walls, borders, spikes, front decorations (with holes)
		std::vector<int> m_frontSpan;       // per cache row: the first and last+1 visible column of the front layer
		int m_cacheW = 0, m_cacheH = 0;
		int m_cachePass = 0;                // Draw: 0 everything, 1 the background layer, 2 the front layer
		size_t m_cacheCapacity = 0;
		const Levels::PlacedRoom *m_cacheRoom = nullptr;
		bool m_cacheOpaque = false;
		bool m_buildingCache = false;   // Draw leaves out the animated decorations
		// A still variant picked at random is not; a hovering rock moves anyway.
		static bool Animated(const DS2DEngine::GameObj *o) { return o->IsHovering() || (o->TotalFrames() > 1 && o->IsAnimating()); }
		// The two cloud layers, tinted once (blue night tints)
		Texture m_clouds[2] = {};
		int m_cloudsW = 0, m_cloudsH = 0;
	};
}
