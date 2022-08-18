#pragma once

#include <Factory/IWidgetSelectorStrategy.h>

namespace factory
{
	class Vector3DWidgetSelectorStrategy : public IWidgetSelectorStrategy
	{
	public:
		bool isAcceptableFor(const types::QGlacierValue & value) override;
		void createAndPlace(const types::QGlacierValue & value, QWidget *owner) override;

		static Vector3DWidgetSelectorStrategy s_instance;
	};
}