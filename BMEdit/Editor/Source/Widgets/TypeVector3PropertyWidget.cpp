#include <Widgets/TypeVector3PropertyWidget.h>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPainter>

#include <Utils/TSpinboxFactory.hpp>

using namespace widgets;
using gamelib::prp::PRPOpCode;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPInstruction;

constexpr const char* X_SPINBOX_ID = "SpinBox_X";
constexpr const char* Y_SPINBOX_ID = "SpinBox_Y";
constexpr const char* Z_SPINBOX_ID = "SpinBox_Z";

enum VectorComponent : int { X = 1, Y = 2, Z = 3 };

void TypeVector3PropertyWidget::buildLayout(const types::QGlacierValue &value)
{
	auto layout = new QHBoxLayout(this);

	switch (value.instructions[1].getOpCode())
	{
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
	    {
		    auto onValueChanged = [this](int entryIdx, int8_t newValue)
		    {
			    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    };

		    utils::TSpinboxFactory<int8_t>::createAndSetup(X_SPINBOX_ID, value, VectorComponent::X, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int8_t>::createAndSetup(Y_SPINBOX_ID, value, VectorComponent::Y, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int8_t>::createAndSetup(Z_SPINBOX_ID, value, VectorComponent::Z, this, layout, onValueChanged);
	    }
		    break;
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
	    {
		    auto onValueChanged = [this](int entryIdx, int16_t newValue)
		    {
			    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    };

		    utils::TSpinboxFactory<int16_t>::createAndSetup(X_SPINBOX_ID, value, VectorComponent::X, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int16_t>::createAndSetup(Y_SPINBOX_ID, value, VectorComponent::Y, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int16_t>::createAndSetup(Z_SPINBOX_ID, value, VectorComponent::Z, this, layout, onValueChanged);
	    }
	        break;
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
	    {
		    auto onValueChanged = [this](int entryIdx, int32_t newValue)
		    {
			    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    };

		    utils::TSpinboxFactory<int32_t>::createAndSetup(X_SPINBOX_ID, value, VectorComponent::X, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int32_t>::createAndSetup(Y_SPINBOX_ID, value, VectorComponent::Y, this, layout, onValueChanged);
		    utils::TSpinboxFactory<int32_t>::createAndSetup(Z_SPINBOX_ID, value, VectorComponent::Z, this, layout, onValueChanged);
	    }
			break;
		case PRPOpCode::Float32:
		case PRPOpCode::NamedFloat32:
	    {
		    auto onValueChanged = [this](int entryIdx, float newValue)
		    {
			    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    };

		    utils::TSpinboxFactory<float>::createAndSetup(X_SPINBOX_ID, value, VectorComponent::X, this, layout, onValueChanged);
		    utils::TSpinboxFactory<float>::createAndSetup(Y_SPINBOX_ID, value, VectorComponent::Y, this, layout, onValueChanged);
		    utils::TSpinboxFactory<float>::createAndSetup(Z_SPINBOX_ID, value, VectorComponent::Z, this, layout, onValueChanged);
	    }
			break;
		case PRPOpCode::Float64:
		case PRPOpCode::NamedFloat64:
	    {
		    auto onValueChanged = [this](int entryIdx, double newValue)
		    {
			    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    };

		    utils::TSpinboxFactory<double>::createAndSetup(X_SPINBOX_ID, value, VectorComponent::X, this, layout, onValueChanged);
		    utils::TSpinboxFactory<double>::createAndSetup(Y_SPINBOX_ID, value, VectorComponent::Y, this, layout, onValueChanged);
		    utils::TSpinboxFactory<double>::createAndSetup(Z_SPINBOX_ID, value, VectorComponent::Z, this, layout, onValueChanged);
	    }
			break;
		default:
			return;
	}

	setLayout(layout);
}

void TypeVector3PropertyWidget::updateLayout(const types::QGlacierValue &value)
{
	switch (value.instructions[1].getOpCode())
	{
	case PRPOpCode::Int8:
	case PRPOpCode::NamedInt8:
	{
		utils::TSpinboxFactory<int8_t>::updateValue(X_SPINBOX_ID, value, VectorComponent::X, this);
		utils::TSpinboxFactory<int8_t>::updateValue(Y_SPINBOX_ID, value, VectorComponent::Y, this);
		utils::TSpinboxFactory<int8_t>::updateValue(Z_SPINBOX_ID, value, VectorComponent::Z, this);
	}
	break;
	case PRPOpCode::Int16:
	case PRPOpCode::NamedInt16:
	{
		utils::TSpinboxFactory<int16_t>::updateValue(X_SPINBOX_ID, value, VectorComponent::X, this);
		utils::TSpinboxFactory<int16_t>::updateValue(Y_SPINBOX_ID, value, VectorComponent::Y, this);
		utils::TSpinboxFactory<int16_t>::updateValue(Z_SPINBOX_ID, value, VectorComponent::Z, this);
	}
	break;
	case PRPOpCode::Int32:
	case PRPOpCode::NamedInt32:
	{
		utils::TSpinboxFactory<int32_t>::updateValue(X_SPINBOX_ID, value, VectorComponent::X, this);
		utils::TSpinboxFactory<int32_t>::updateValue(Y_SPINBOX_ID, value, VectorComponent::Y, this);
		utils::TSpinboxFactory<int32_t>::updateValue(Z_SPINBOX_ID, value, VectorComponent::Z, this);
	}
	break;
	case PRPOpCode::Float32:
	case PRPOpCode::NamedFloat32:
	{
		utils::TSpinboxFactory<float>::updateValue(X_SPINBOX_ID, value, VectorComponent::X, this);
		utils::TSpinboxFactory<float>::updateValue(Y_SPINBOX_ID, value, VectorComponent::Y, this);
		utils::TSpinboxFactory<float>::updateValue(Z_SPINBOX_ID, value, VectorComponent::Z, this);
	}
	break;
	case PRPOpCode::Float64:
	case PRPOpCode::NamedFloat64:
	{
		utils::TSpinboxFactory<double>::updateValue(X_SPINBOX_ID, value, VectorComponent::X, this);
		utils::TSpinboxFactory<double>::updateValue(Y_SPINBOX_ID, value, VectorComponent::Y, this);
		utils::TSpinboxFactory<double>::updateValue(Z_SPINBOX_ID, value, VectorComponent::Z, this);
	}
	break;
	default:
		return;
	}
}


void TypeVector3PropertyWidget::paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data)
{
	QTextOption textOption;
	textOption.setAlignment(Qt::AlignCenter);

	QString text;
	const QString format("(%1; %2; %3)");
	const auto& x = data.instructions[1].getOperand();
	const auto& y = data.instructions[2].getOperand();
	const auto& z = data.instructions[3].getOperand();

	switch (data.instructions[1].getOpCode())
	{
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
			text = format
			           .arg(x.trivial.i32)
			           .arg(y.trivial.i32)
			           .arg(z.trivial.i32);
			break;
		case PRPOpCode::Float32:
		case PRPOpCode::NamedFloat32:
			text = format
			           .arg(x.trivial.f32)
			           .arg(y.trivial.f32)
			           .arg(z.trivial.f32);
			break;
		case PRPOpCode::Float64:
		case PRPOpCode::NamedFloat64:
			text = format
			           .arg(x.trivial.f64)
			           .arg(y.trivial.f64)
			           .arg(z.trivial.f64);
			break;
		default:
			return;
	}

	painter->drawText(option.rect, text, textOption);
}