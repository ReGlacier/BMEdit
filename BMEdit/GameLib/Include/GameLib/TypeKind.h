#pragma once

#include <cstdint>
#include <string>


namespace gamelib
{
	enum class TypeKind : uint8_t
	{
		ENUM = 0, ///< Enumeration
		ALIAS = 1, ///< Alias to another type or alias
		COMPLEX = 2, ///< Complex type (class or data)
		ARRAY = 3, ///< Array of values of same type
		CONTAINER = 4, ///< Container for objects
		RAW_DATA = 5, ///< Raw data (array of bytes)
		BITFIELD = 6, ///< Bit field (integer with declared options)
		NONE, ///< Initial value
	};

	[[nodiscard]] TypeKind fromString(const std::string &typeKindStr);
}