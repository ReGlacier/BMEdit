#include <QLabel>
#include <QLayout>
#include <Widgets/TypePropertyWidget.h>


namespace widgets {
	TypePropertyWidget::TypePropertyWidget(QWidget* parent)
	    : QWidget(parent)
	{
		setAutoFillBackground(true);
	}

	void TypePropertyWidget::setValue(const types::QGlacierValue &value)
	{
		m_value = value;
	}

	const types::QGlacierValue &TypePropertyWidget::getValue() const
	{
		return m_value;
	}
}