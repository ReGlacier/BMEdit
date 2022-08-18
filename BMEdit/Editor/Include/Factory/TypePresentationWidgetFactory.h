#pragma once

#include "Types/QGlacierValue.h"
#include <QWidget>


namespace factory
{
	struct TypePresentationWidgetFactory
	{
		static void produceAndPlace(const types::QGlacierValue& value, QWidget *owner);
	};
}