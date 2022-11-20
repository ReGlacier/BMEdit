#pragma once

#include <GameLib/Value.h>
#include <QMetaType>
#include <QString>

namespace types
{
	struct QGlacierController
	{
		int uniqueId { 0 };  // Assigned when created
		QString name;        // Name of the controller
		gamelib::Value data; // Data of the controller
	};
}

Q_DECLARE_METATYPE(types::QGlacierController);