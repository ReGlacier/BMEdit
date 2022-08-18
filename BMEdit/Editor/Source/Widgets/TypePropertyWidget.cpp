#include <Widgets/TypePropertyWidget.h>
#include <Factory/TypePresentationWidgetFactory.h>
#include <QLayout>


namespace widgets {
	TypePropertyWidget::TypePropertyWidget(QWidget* parent) : QWidget(parent) {}

	void TypePropertyWidget::setValue(const types::QGlacierValue &value)
	{
		m_value = value;
		rebuildLayout();
	}

	void TypePropertyWidget::updateValue(const types::QGlacierValue &value)
	{
		m_value = value;

		emit valueChanged();
	}

	const types::QGlacierValue &TypePropertyWidget::getValue() const
	{
		return m_value;
	}

	void TypePropertyWidget::rebuildLayout()
	{
		clearLayout();

		// Type hinters
		if (m_value.instructions.empty() || m_value.views.empty())
			return;

		factory::TypePresentationWidgetFactory::produceAndPlace(m_value, this);
	}

	void TypePropertyWidget::clearLayout()
	{
		// Clear layout
		if (layout())
		{
			while (layout()->count() > 0)
			{
				QLayoutItem* item = layout()->takeAt(0);
				QWidget* widget = item->widget();

				delete widget;
				delete item;
			}
		}
	}
}