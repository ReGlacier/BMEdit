#include <Widgets/TypeMatrixPropertyWidget.h>
#include <GameLib/PRP/PRPOpCode.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <Utils/TSpinBoxFactory.hpp>

#include <QGridLayout>
#include <QTextOption>
#include <QPainter>

using namespace widgets;
using gamelib::prp::PRPInstruction;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPOpCode;

constexpr const char* MATRIX_COMPONENT_ID = "%1_MATRIX_COMPONENT_SPINBOX_ID";


TypeMatrixPropertyWidget::TypeMatrixPropertyWidget(int rows, int columns, QWidget *parent)
    : TypePropertyWidget(parent)
    , m_rows(rows)
    , m_columns(columns)
{
	assert(m_rows > 0);
	assert(m_columns > 0);
}

void TypeMatrixPropertyWidget::buildLayout(const types::QGlacierValue &data)
{
	auto layout = new QGridLayout(this);

	for (int total = 1, row = 0; row < m_rows; ++row)
	{
		for (int column = 0; column < m_columns; ++column)
		{
			switch (data.instructions[total].getOpCode())
			{
				case PRPOpCode::Int8:
				case PRPOpCode::NamedInt8:
			    {
				    auto onDataChanged = [this](int entryIdx, int8_t newValue) {
					    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					    valueChanged();
				    };
				    auto widget = utils::TSpinboxFactory<int8_t>::create(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this, onDataChanged);
				    layout->addWidget(widget, row, column);
			    }
				    break;
				case PRPOpCode::Int16:
				case PRPOpCode::NamedInt16:
			    {
				    auto onDataChanged = [this](int entryIdx, int16_t newValue) {
					    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					    valueChanged();
				    };
				    auto widget = utils::TSpinboxFactory<int16_t>::create(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this, onDataChanged);
				    layout->addWidget(widget, row, column);
			    }
				case PRPOpCode::Int32:
				case PRPOpCode::NamedInt32:
			    {
				    auto onDataChanged = [this](int entryIdx, int32_t newValue) {
					    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					    valueChanged();
				    };
				    auto widget = utils::TSpinboxFactory<int32_t>::create(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this, onDataChanged);
				    layout->addWidget(widget, row, column);
			    }
				    break;
				case PRPOpCode::Float32:
				case PRPOpCode::NamedFloat32:
			    {
				    auto onDataChanged = [this](int entryIdx, float newValue) {
					    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					    valueChanged();
				    };
				    auto widget = utils::TSpinboxFactory<float>::create(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this, onDataChanged);
				    layout->addWidget(widget, row, column);
			    }
				    break;
				case PRPOpCode::Float64:
				case PRPOpCode::NamedFloat64:
			    {
				    auto onDataChanged = [this](int entryIdx, double newValue) {
					    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					    valueChanged();
				    };
				    auto widget = utils::TSpinboxFactory<double>::create(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this, onDataChanged);
				    layout->addWidget(widget, row, column);
			    }
					break;
				default:
					assert(false);
					return;
			}

			++total;
		}
	}
}

void TypeMatrixPropertyWidget::updateLayout(const types::QGlacierValue &data)
{
	for (int total = 1, row = 0; row < m_rows; ++row)
	{
		for (int column = 0; column < m_columns; ++column)
		{
			switch (data.instructions[total].getOpCode())
			{
				case PRPOpCode::Int8:
				case PRPOpCode::NamedInt8:
				    utils::TSpinboxFactory<int8_t>::updateValue(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this);
					break;
				case PRPOpCode::Int16:
				case PRPOpCode::NamedInt16:
				    utils::TSpinboxFactory<int16_t>::updateValue(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this);
				    break;
				case PRPOpCode::Int32:
				case PRPOpCode::NamedInt32:
				    utils::TSpinboxFactory<int32_t>::updateValue(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this);
				    break;
				case PRPOpCode::Float32:
				case PRPOpCode::NamedFloat32:
				    utils::TSpinboxFactory<float>::updateValue(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this);
				    break;
				case PRPOpCode::Float64:
				case PRPOpCode::NamedFloat64:
				    utils::TSpinboxFactory<double>::updateValue(QString(MATRIX_COMPONENT_ID).arg(total), data, total, this);
				    break;
				default:
					assert(false);
					return;
			}

			++total;
		}
	}
}

void TypeMatrixPropertyWidget::paintPreview(int rows, int columns, QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data)
{
	if (rows <= 0 || columns <= 0)
	{
		assert(rows > 0);
		assert(columns > 0);
		return;
	}

	// School code style, I know, bruuh, who cares? :)
	QTextOption textOption;
	textOption.setAlignment(Qt::AlignCenter);

	QString text;

	for (int total = 1, i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			switch (data.instructions[total].getOpCode())
			{
				case PRPOpCode::Int8:
				case PRPOpCode::NamedInt8:
				case PRPOpCode::Int16:
				case PRPOpCode::NamedInt16:
				case PRPOpCode::Int32:
				case PRPOpCode::NamedInt32:
				    text.push_back(QString("%1 ").arg(data.instructions[total].getOperand().get<int32_t>()));
					break;
				case PRPOpCode::Float32:
				case PRPOpCode::NamedFloat32:
				    text.push_back(QString("%1 ").arg(data.instructions[total].getOperand().get<float>()));
					break;
				case PRPOpCode::Float64:
				case PRPOpCode::NamedFloat64:
					text.push_back(QString("%1 ").arg(data.instructions[total].getOperand().get<double>()));
					break;
				default:
				    assert(false);
					return;
			}

			++total;
		}

		text.push_back('\n');
	}

	painter->drawText(option.rect, text, textOption);
}