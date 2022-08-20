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
		if (!areSame(m_value, value))
		{
			buildLayout(value);
		}
		else
		{
			updateLayout(value);
		}

		m_value = value;
	}

	const types::QGlacierValue &TypePropertyWidget::getValue() const
	{
		return m_value;
	}

	bool TypePropertyWidget::areSame(const types::QGlacierValue &current, const types::QGlacierValue &value)
	{
		if (current.instructions.size() != value.instructions.size()) return false;

		for (auto instructionIndex = 0; instructionIndex < current.instructions.size(); ++instructionIndex)
		{
			const auto& c = current.instructions[instructionIndex];
			const auto& n = value.instructions[instructionIndex];

			if (c.getOpCode() != n.getOpCode()) return false;
		}

		return true;
	}

	void TypePropertyWidget::buildLayout(const types::QGlacierValue &value)
	{// stub
	}

	void TypePropertyWidget::updateLayout(const types::QGlacierValue &value)
	{// stub
	}
}