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
		gamelib::Value value {};
	};
}

Q_DECLARE_METATYPE(types::QGlacierValue)