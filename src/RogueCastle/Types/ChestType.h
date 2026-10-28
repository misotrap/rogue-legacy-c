/* Port of RogueCastle/src/Types/ChestType.cs: the chest colours. */
#ifndef ROGUE_CASTLE_CHEST_TYPE_H
#define ROGUE_CASTLE_CHEST_TYPE_H

#include <cstdint>

namespace RogueCastle
{
namespace ChestType
{
	constexpr uint8_t None = 0;
	constexpr uint8_t Brown = 1;
	constexpr uint8_t Silver = 2;
	constexpr uint8_t Gold = 3;
	constexpr uint8_t Fairy = 4;
	constexpr uint8_t Boss = 5;
}
}

#endif
