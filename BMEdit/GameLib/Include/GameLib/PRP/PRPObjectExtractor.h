#pragma once

#include <GameLib/Span.h>
#include <GameLib/PRP/PRPInstruction.h>


namespace gamelib
{
	class Value;

	template <typename T>
	struct TObjectExtractor
	{
		static T extract(const Span<prp::PRPInstruction>& instructions);
	};

	template <typename T>
	concept HasSpecializationTObjectExtractor = requires {
		typename TObjectExtractor<T>;
	};

}