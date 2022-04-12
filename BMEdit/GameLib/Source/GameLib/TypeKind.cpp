#include <GameLib/TypeKind.h>
#include <algorithm>
#include <iterator>


namespace gamelib
{
	TypeKind fromString(const std::string &typeKindStr)
	{
		std::string str;
		bool isBeforeDot = true;
		std::transform(
			typeKindStr.begin(),
			typeKindStr.end(),
			std::back_inserter(str),
			[&isBeforeDot](char ch)
			{
				if (ch == '.')
				{
					isBeforeDot = false;
				}

				if (isBeforeDot)
				{
					return ch;
				}

				return (char)std::toupper(ch);
			});

		if (!isBeforeDot) {
			// Old form (Python compatible; TypeKind.<KIND>)
			if (str == "TypeKind.ENUM") return TypeKind::ENUM;
			else if (str == "TypeKind.ALIAS") return TypeKind::ALIAS;
			else if (str == "TypeKind.COMPLEX") return TypeKind::COMPLEX;
			else if (str == "TypeKind.ARRAY") return TypeKind::ARRAY;
			else if (str == "TypeKind.CONTAINER") return TypeKind::CONTAINER;
			else if (str == "TypeKind.RAW_DATA") return TypeKind::RAW_DATA;
			else if (str == "TypeKind.BITFIELD") return TypeKind::BITFIELD;
		} else {
			// New form (C++ compatible; <KIND>)
			if (str == "ENUM") return TypeKind::ENUM;
			else if (str == "ALIAS") return TypeKind::ALIAS;
			else if (str == "COMPLEX") return TypeKind::COMPLEX;
			else if (str == "ARRAY") return TypeKind::ARRAY;
			else if (str == "CONTAINER") return TypeKind::CONTAINER;
			else if (str == "RAW_DATA") return TypeKind::RAW_DATA;
			else if (str == "BITFIELD") return TypeKind::BITFIELD;
		}

		return TypeKind::NONE;
	}
}