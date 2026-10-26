/* Minimal port of RogueCastle/src/Game.cs: the global game state the object
 * code reaches through the Game class (GlobalInput). */
#ifndef ROGUE_CASTLE_GAME_H
#define ROGUE_CASTLE_GAME_H

#include "../InputSystem/InputManager.h"

namespace RogueCastle
{

class Game
{
public:
	static InputSystem::InputManager GlobalInput;
};

} // namespace RogueCastle

#endif
