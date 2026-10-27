/* Port of RogueCastle/src/Types/InputMapType.cs (subset). The enum values
 * are the platform button bits (platform.h), so InputManager reads them
 * straight from plat_poll. */
#ifndef ROGUE_CASTLE_INPUT_MAP_TYPE_H
#define ROGUE_CASTLE_INPUT_MAP_TYPE_H

#include "../../platform.h"

namespace RogueCastle
{

enum InputMapType
{
	PLAYER_UP1 = BTN_UP,
	PLAYER_UP2 = BTN_UP,
	PLAYER_DOWN1 = BTN_DOWN,
	PLAYER_DOWN2 = BTN_DOWN,
	PLAYER_LEFT1 = BTN_LEFT,
	PLAYER_LEFT2 = BTN_LEFT,
	PLAYER_RIGHT1 = BTN_RIGHT,
	PLAYER_RIGHT2 = BTN_RIGHT,
	PLAYER_JUMP1 = BTN_B,
	PLAYER_JUMP2 = BTN_B,
	PLAYER_ATTACK = BTN_Y,
	PLAYER_BLOCK = BTN_L1,
	PLAYER_DASHLEFT = BTN_L2,   /* C# default: Q key / left trigger */
	PLAYER_DASHRIGHT = BTN_R2,  /* C# default: right trigger */
	PLAYER_SPELL = BTN_A,
	MENU_MAP = BTN_SELECT,
	MENU_PAUSE = BTN_START
};

} // namespace RogueCastle

#endif
