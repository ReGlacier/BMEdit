#include <Widgets/TypeMatrixPropertyWidget.h>
#include <GameLib/PRP/PRPOpCode.h>
#include <GameLib/PRP/PRPInstruction.h>

#include <QTextOption>
#include <QPainter>

using namespace widgets;
using gamelib::prp::PRPInstruction;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPOpCode;


TypeMatrixPropertyWidget::TypeMatrixPropertyWidget(int rows, int columns, QWidget *parent)
    : TypePropertyWidget(parent)
    , m_rows(rows)
    , m_columns(columns)
{
	assert(m_rows > 0);
	assert(m_columns > 0);
}

void TypeMatrixPropertyWidget::buildLayout(const types::QGlacierValue &value)
{
}

void TypeMatrixPropertyWidget::updateLayout(const types::QGlacierValue &value)
{
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