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
		// SkyObj: night sky, moon and two cloud layers, seen through the
		// magenta parts of the backgrounds (ParallaxFX in the original)
		static void DrawSky(DS2DEngine::Camera2D *camera);
	};
}
