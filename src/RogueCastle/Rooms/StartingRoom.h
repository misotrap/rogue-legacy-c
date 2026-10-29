// The courtyard in front of the castle (StartingRoomObj.cs): the castle's
// "Start" room (rooms[0], at 0,0) with the blacksmith and his board, the
// enchantress and her tent, the architect and his gear, each shown once
// its manor skill is bought (Smithy, Enchanter, Architect). Near one of
// them the up-arrow bubble bobs over its head; Up talks to it (the shops,
// the architect's dialogue). Walking out on the left opens the manor (the
// skill screen), as in the original. After the first death the toll
// collector (Charon) stands on the bridge and blocks the way until paid.
// It rains (StartingRoomObj forces m_isRaining): 200 drops (the original's
// 400, halved like LevelEV.SAVE_FRAMES) and the scene 30% darker. The
// architect's lever animation is not ported yet.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../Skills/SkillTree.h"

#include <cstdint>
#include <vector>

namespace RogueCastle
{
	class PlayerObj;

	class StartingRoom
	{
	public:
		enum class Action { None, Blacksmith, Enchantress, Architect, Toll, Manor };

		~StartingRoom();
		bool LoadContent(const char *assetsDir);
		// player: the castle player, inside the start room
		Action Update(PlayerObj *player, const Skills::SkillTree &skills, uint32_t pressed, double dt);
		void Draw(DS2DEngine::Camera2D *camera, const Skills::SkillTree &skills);
		// The rain over everything (after the player): call after Draw
		void DrawRain(uint32_t *frame, DS2DEngine::Camera2D *camera);
		// Back from the manor: the hero walks in again from the left edge
		static void PlaceAtLeftEdge(PlayerObj *player);
		// A new castle run: Charon is back when the family has died before
		void ResetToll(bool available) { m_tollVisible = available; }
		void HideToll() { m_tollVisible = false; }
		bool TollVisible() const { return m_tollVisible; }

	private:
		DS2DEngine::ObjContainer *m_blacksmith = nullptr, *m_enchantress = nullptr, *m_architect = nullptr, *m_toll = nullptr;
		bool m_tollVisible = false;
		struct Drop { float x, y, speedY; };
		std::vector<Drop> m_rain;   // world coordinates
		DS2DEngine::SpriteObj *m_board = nullptr, *m_tent = nullptr, *m_gear = nullptr, *m_arrow = nullptr;
		DS2DEngine::GameObj *m_near = nullptr;   // the NPC the arrow is over
		double m_time = 0;
	};
}
