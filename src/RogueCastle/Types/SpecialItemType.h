/* Port of RogueCastle/src/Types/SpecialItemType.cs: the one special item
 * the hero carries (from the bonus rooms, the carnival tokens...). The
 * save field is a raw byte; these constants name the values. */
#ifndef ROGUE_CASTLE_SPECIAL_ITEM_TYPE_H
#define ROGUE_CASTLE_SPECIAL_ITEM_TYPE_H

#include <cstdint>

namespace RogueCastle
{
namespace SpecialItemType
{
	constexpr uint8_t None = 0;
	constexpr uint8_t FreeEntrance = 1;
	constexpr uint8_t LoseCoins = 2;
	constexpr uint8_t Revive = 3;
	constexpr uint8_t SpikeImmunity = 4;
	constexpr uint8_t GoldPerKill = 5;
	constexpr uint8_t Compass = 6;

	constexpr uint8_t Total = 7;

	constexpr uint8_t Glasses = 8;   // not in the random list (hard coded)

	constexpr uint8_t EyeballToken = 9;
	constexpr uint8_t SkullToken = 10;
	constexpr uint8_t FireballToken = 11;
	constexpr uint8_t BlobToken = 12;
	constexpr uint8_t LastBossToken = 13;
}
}

#endif
