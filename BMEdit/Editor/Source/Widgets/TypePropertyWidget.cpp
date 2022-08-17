#include <Widgets/TypePropertyWidget.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLayout>

namespace widgets {
	TypePropertyWidget::TypePropertyWidget(QWidget* parent) : QWidget(parent) {}

	void TypePropertyWidget::setValue(const types::QGlacierValue &value) { m_value = value; rebuildLayout(); }
	const types::QGlacierValue &TypePropertyWidget::getValue() const { return m_value; }

	QSize TypePropertyWidget::sizeHint() const
	{
		return m_minSize;
	}

	void TypePropertyWidget::rebuildLayout()
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

		auto newLayout = new QVBoxLayout(this);
		setLayout(newLayout);

		//FIXME: Here we need to implement our factory
		m_minSize = QSize(100, 100);
		auto btn = new QPushButton("HELLO", this);
		newLayout->addWidget(btn);
	}
}