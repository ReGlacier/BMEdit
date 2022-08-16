#include <Widgets/TypePropertyWidget.h>

namespace widgets {
	TypePropertyWidget::TypePropertyWidget(QWidget* parent) : QWidget(parent) {}

	void TypePropertyWidget::setValue(const types::QGlacierValue &value) { m_value = value; }
	const types::QGlacierValue &TypePropertyWidget::getValue() const { return m_value; }

	QSize TypePropertyWidget::sizeHint() const
	{
		return QSize(20, 20);
	}

	void TypePropertyWidget::paintEvent(QPaintEvent *event)
	{

	}
}