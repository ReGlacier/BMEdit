#include <Factory/TypePresentationWidgetFactory.h>
#include <Factory/IWidgetSelectorStrategy.h>
#include <Factory/Vector3DWidgetSelectorStrategy.h>
#include <QHBoxLayout>

using namespace factory;


void TypePresentationWidgetFactory::produceAndPlace(const types::QGlacierValue& value, QWidget *owner)
{
	// Put your custom strategies
	static factory::IWidgetSelectorStrategy* s_creationStrategyList[] = {
	    &factory::Vector3DWidgetSelectorStrategy::s_instance,
	};

	if (value.views.empty() || value.instructions.empty())
		return;

	for (const auto& strategy: s_creationStrategyList)
	{
		if (strategy->isAcceptableFor(value))
		{
			strategy->createAndPlace(value, owner);
			break;
		}
	}
}