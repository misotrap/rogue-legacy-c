// The courtyard in front of the castle (StartingRoomObj.cs): the castle's
// "Start" room (rooms[0], at 0,0) with the blacksmith and his board, the
// enchantress and her tent, the architect and his gear, each shown once
// its manor skill is bought (Smithy, Enchanter, Architect). Near one of
// them an up arrow bobs over its head; Up talks to it. Walking out on the
// left opens the manor (the skill screen), as in the original. The toll
// collector, the rain and the architect's dialogue are not ported yet.
#pragma once

#include "../../DS2DEngine/Camera2D.h"
#include "../../DS2DEngine/GameObjs/ObjContainer.h"
#include "../../DS2DEngine/GameObjs/SpriteObj.h"
#include "../Skills/SkillTree.h"

#include <cstdint>

namespace RogueCastle
{
	class PlayerObj;

	class StartingRoom
	{
	public:
		enum class Action { None, Blacksmith, Enchantress, Manor };

		~StartingRoom();
		bool LoadContent(const char *assetsDir);
		// player: the castle player, inside the start room
		Action Update(PlayerObj *player, const Skills::SkillTree &skills, uint32_t pressed, double dt);
		void Draw(DS2DEngine::Camera2D *camera, const Skills::SkillTree &skills);
		// Back from the manor: the hero walks in again from the left edge
		static void PlaceAtLeftEdge(PlayerObj *player);

	private:
		DS2DEngine::ObjContainer *m_blacksmith = nullptr, *m_enchantress = nullptr, *m_architect = nullptr;
		DS2DEngine::SpriteObj *m_board = nullptr, *m_tent = nullptr, *m_gear = nullptr, *m_arrow = nullptr;
		DS2DEngine::GameObj *m_near = nullptr;   // the NPC the arrow is over
		double m_time = 0;
	};
}
