#pragma once

#include <QWidget>
#include <Types/QGlacierValue.h>

namespace factory
{
	class IWidgetSelectorStrategy
	{
	public:
		virtual ~IWidgetSelectorStrategy() noexcept = default;

		virtual bool isAcceptableFor(const types::QGlacierValue & value) = 0;
		virtual void createAndPlace(const types::QGlacierValue & value, QWidget *owner) = 0;
	};
}