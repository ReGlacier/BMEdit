#pragma once

#include <cstdint>


namespace gamelib::gms
{
	enum GMSSectionOffsets : uint32_t
	{
		ENTITIES = 0x0,
		GEOM_STATS = 0x10,
		EVENTS_DATA = 0x1C,
		MATERIALS = 0x30,
		PATH_FINDER_DATA = 0x34,
		LEGACY_PHYSICS_DATA = 0x38,
		WEAPON_HANDLES = 0x40,
		EXCLUDED_ANIMATIONS_LIST = 0x44
	};
}