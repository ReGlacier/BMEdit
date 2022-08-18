#pragma once

#include "Types/QGlacierValue.h"
#include <QVariant>


namespace editor
{
	class PropertyAsStringRepr
	{
	public:
		static QVariant getStringRepresentationOfValue(const types::QGlacierValue& value);
	};
}