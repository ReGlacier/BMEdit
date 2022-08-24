#pragma once

#include <QMetaType>
#include <GameLib/PRP/PRPDefinition.h>

namespace types
{
	struct QSceneProperty
	{
		gamelib::prp::PRPDefinition def;
	};
}

Q_DECLARE_METATYPE(types::QSceneProperty)