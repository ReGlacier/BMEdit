#pragma once

#include <QMetaType>
#include <GameLib/Value.h>


namespace types
{
	/**
	 * Just hold a gamelib::Value thing
	 */
	struct QGlacierValue
	{
		std::vector<gamelib::prp::PRPInstruction> instructions;
		std::vector<gamelib::ValueView> views;
	};
}

Q_DECLARE_METATYPE(types::QGlacierValue)