// The reward screen (GetItemScreen.cs): a blueprint, a rune or the stat
// bonuses of a chest rise from it, shrink, then grow in the middle of the
// screen over the gold "found" plate, with the item's name and sparkles.
// Any button closes it once the animation is done. The chests queue their
// rewards with GetItem::Push; GameFlow opens the screen for each one and
// the game waits behind it.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../../Text/Font.h"

#include <cstdint>
#include <string>

namespace RogueCastle
{
	namespace GetItem
	{
		enum Type { Blueprint = 1, Rune = 2, StatDrop = 3, Spell = 4, SpecialItem = 5, TripStatDrop = 6 };   // GetItemType
		struct Reward
		{
			int type = 0;
			int x = 0, y = 0;       // m_itemInfo: (category, index) or (stat drop type, -)
			int stat2 = 0, stat3 = 0;   // m_tripStatData
			float worldX = 0, worldY = 0;   // where the item rises from (the chest)
		};
		void Push(const Reward &reward);
		bool Pop(Reward &reward);
	}

	class GetItemScreen
	{
	public:
		~GetItemScreen();
		bool LoadContent(const char *assetsDir);
		// camX/camY: the level camera, to start from the chest on screen
		void Open(const GetItem::Reward &reward, float camX, float camY, int timesCastleBeaten);
		// false once closed
		bool Update(uint32_t pressed, double dt);
		void Draw(uint32_t *frame);

	private:
		static const int PARTICLES = 10;
		GetItem::Reward m_reward;
		Text::Font m_font;
		DS2DEngine::Camera2D m_camera;   // the 1320x720 screen at zoom 0.5
		DS2DEngine::SpriteObj *m_bg = nullptr, *m_found = nullptr, *m_items[3] = {};
		DS2DEngine::SpriteObj *m_particles[PARTICLES] = {};
		float m_particleX[PARTICLES], m_particleY[PARTICLES], m_particleDelay[PARTICLES];
		std::string m_texts[3];
		float m_startX = 0, m_startY = 0, m_rotation = 0;
		double m_time = 0, m_exitTime = -1;
		int m_count = 1;
	};
}
