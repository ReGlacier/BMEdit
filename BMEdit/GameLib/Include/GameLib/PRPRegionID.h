#pragma once

namespace gamelib::prp {
	enum class PRPRegionID {
		HEADER,
		TOKEN_TABLE,
		ZDEFINITIONS,
		INSTRUCTIONS,
		UNKNOWN
	};
}