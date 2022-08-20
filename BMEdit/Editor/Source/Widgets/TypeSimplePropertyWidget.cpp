#include <Widgets/TypeSimplePropertyWidget.h>
#include <QApplication>
#include <QPainter>

#include <GameLib/Type.h>
#include <GameLib/TypeEnum.h>

// Layout
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// Trivial widgets
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

// Models
#include <QStringListModel>

// Namespaces
using namespace widgets;

using gamelib::prp::PRPInstruction;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPOpCode;

// Constants
constexpr const char* BOOL_CHECKBOX_ID = "BoolCheckBoxID";
constexpr const char* INT_SPINBOX_ID = "IntSpinBoxID";
constexpr const char* F32_SPINBOX_ID = "Float32SpinBoxID";
constexpr const char* F64_SPINBOX_ID = "Float64SpinBoxID";
constexpr const char* STR_LINE_EDIT_ID = "StringLineEditorID";
constexpr const char* ENUM_COMBOBOX_ID = "EnumEditorID";


void TypeSimplePropertyWidget::buildLayout(const types::QGlacierValue &value)
{
	if (value.instructions[0].isBool())
	{
		createCheckBoxLayout(value);
	}
	else if (value.instructions[0].isNumber())
	{
		createNumberLayout(value);
	}
	else if (value.instructions[0].isString())
	{
		createStringLayout(value);
	}
	else if (value.instructions[0].isEnum())
	{
		createEnumLayout(value);
	}

	if (auto layout = this->layout())
	{
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);

#if 0 // IDK is it helpful or not, just don't touch
		for (int i = 0; i < layout->count(); ++i)
		{
			auto item = layout->itemAt(i);
			if (auto widget = item->widget())
			{
				widget->setContentsMargins(0, 0, 0, 0);
				widget->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
				break;
			}
		}
#endif
	}
}

void TypeSimplePropertyWidget::updateLayout(const types::QGlacierValue &value)
{
	if (value.instructions[0].isBool())
	{
		updateCheckBoxLayout(value);
	}
	else if (value.instructions[0].isNumber())
	{
		updateNumberLayout(value);
	}
	else if (value.instructions[0].isString())
	{
		updateStringLayout(value);
	}
	else if (value.instructions[0].isEnum())
	{
		updateEnumLayout(value);
	}
}

void TypeSimplePropertyWidget::createCheckBoxLayout(const types::QGlacierValue &value)
{
	auto layout = new QHBoxLayout(this);
	auto checkbox = new QCheckBox(this);

	checkbox->setChecked(value.instructions[0].getOperand().trivial.b);
	connect(checkbox, &QCheckBox::stateChanged, [this](int state) {
		if (state == Qt::CheckState::Checked)
		{
			m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(true));
		}
		else if (state == Qt::CheckState::Unchecked)
		{
			m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(false));
		}

		valueChanged();
	});

	checkbox->setAccessibleName(BOOL_CHECKBOX_ID);
	layout->addWidget(checkbox);
	setLayout(layout);
}

void TypeSimplePropertyWidget::updateCheckBoxLayout(const types::QGlacierValue &value)
{
	if (auto checkbox = findChild<QCheckBox*>(BOOL_CHECKBOX_ID))
	{
		QSignalBlocker blocker(checkbox);
		checkbox->setChecked(value.instructions[0].getOperand().trivial.b);
	}
}

void TypeSimplePropertyWidget::createNumberLayout(const types::QGlacierValue &value)
{
	auto layout = new QHBoxLayout(this);

	switch (value.instructions[0].getOpCode())
	{
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
		{
		    auto spinBox = new QSpinBox(this);
		    spinBox->setMinimum(std::numeric_limits<int8_t>::lowest());
		    spinBox->setMaximum(std::numeric_limits<int8_t>::max());
		    spinBox->setValue(value.instructions[0].getOperand().trivial.i8);
		    spinBox->setAccessibleName(INT_SPINBOX_ID);
		    connect(spinBox, &QSpinBox::valueChanged, [this](int newValue) {
			    m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(static_cast<int8_t>(newValue)));
			    valueChanged();
		    });
		    layout->addWidget(spinBox);
		}
		break;
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
	    {
		    auto spinBox = new QSpinBox(this);
		    spinBox->setMinimum(std::numeric_limits<int16_t>::lowest());
		    spinBox->setMaximum(std::numeric_limits<int16_t>::max());
		    spinBox->setValue(value.instructions[0].getOperand().trivial.i16);
		    spinBox->setAccessibleName(INT_SPINBOX_ID);
		    connect(spinBox, &QSpinBox::valueChanged, [this](int newValue) {
			    m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(static_cast<int16_t>(newValue)));
			    valueChanged();
		    });
		    layout->addWidget(spinBox);
	    }
		break;
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
	    {
		    auto spinBox = new QSpinBox(this);
		    spinBox->setMinimum(std::numeric_limits<int32_t>::lowest());
		    spinBox->setMaximum(std::numeric_limits<int32_t>::max());
		    spinBox->setValue(value.instructions[0].getOperand().trivial.i32);
		    spinBox->setAccessibleName(INT_SPINBOX_ID);
		    connect(spinBox, &QSpinBox::valueChanged, [this](int newValue) {
			    m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(static_cast<int32_t>(newValue)));
			    valueChanged();
		    });
		    layout->addWidget(spinBox);
	    }
		break;
		case PRPOpCode::Float32:
		case PRPOpCode::NamedFloat32:
	    {
		    auto spinBox = new QDoubleSpinBox(this);
		    spinBox->setMinimum(std::numeric_limits<float>::lowest());
		    spinBox->setMaximum(std::numeric_limits<float>::max());
		    spinBox->setValue(value.instructions[0].getOperand().trivial.f32);
		    spinBox->setAccessibleName(F32_SPINBOX_ID);
		    connect(spinBox, &QDoubleSpinBox::valueChanged, [this](double newValue) {
			    m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(static_cast<float>(newValue)));
			    valueChanged();
		    });
		    layout->addWidget(spinBox);
	    }
		break;
		case PRPOpCode::Float64:
		case PRPOpCode::NamedFloat64:
	    {
		    auto spinBox = new QDoubleSpinBox(this);
		    spinBox->setMinimum(std::numeric_limits<double>::lowest());
		    spinBox->setMaximum(std::numeric_limits<double>::max());
		    spinBox->setValue(value.instructions[0].getOperand().trivial.f64);
		    spinBox->setAccessibleName(F64_SPINBOX_ID);
		    connect(spinBox, &QDoubleSpinBox::valueChanged, [this](double newValue) {
			    m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(newValue));
			    valueChanged();
		    });
		    layout->addWidget(spinBox);
	    }
		break;
		default:
			assert(false);
			return;
	}

	setLayout(layout);
}

void TypeSimplePropertyWidget::updateNumberLayout(const types::QGlacierValue &value)
{
	switch (value.instructions[0].getOpCode())
	{
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
		{
		    if (auto spinBox = findChild<QSpinBox*>(INT_SPINBOX_ID))
		    {
			    QSignalBlocker blocker(spinBox);
			    spinBox->setValue(value.instructions[0].getOperand().trivial.i32);
		    }
		}
		break;
		case PRPOpCode::Float32:
		case PRPOpCode::NamedFloat32:
	    {
		    if (auto spinBox = findChild<QDoubleSpinBox*>(F32_SPINBOX_ID))
		    {
			    QSignalBlocker blocker(spinBox);
			    spinBox->setValue(value.instructions[0].getOperand().trivial.f32);
		    }
	    }
	    break;
		case PRPOpCode::Float64:
		case PRPOpCode::NamedFloat64:
		{
		    if (auto spinBox = findChild<QDoubleSpinBox*>(F64_SPINBOX_ID))
		    {
			    QSignalBlocker blocker(spinBox);
			    spinBox->setValue(value.instructions[0].getOperand().trivial.f64);
		    }
		}
		break;
		default:
			assert(false);
			return;
	}
}

void TypeSimplePropertyWidget::createStringLayout(const types::QGlacierValue &value)
{
	auto layout = new QHBoxLayout(this);

	auto lineEdit = new QLineEdit(this);
	lineEdit->setText(QString::fromStdString(value.instructions[0].getOperand().str));
	connect(lineEdit, &QLineEdit::textChanged, [this](const QString &newValue) {
		m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(newValue.toStdString()));
		valueChanged();
	});

	layout->addWidget(lineEdit);
	setLayout(layout);
}

void TypeSimplePropertyWidget::updateStringLayout(const types::QGlacierValue &value)
{
	if (auto lineEdit = findChild<QLineEdit*>(STR_LINE_EDIT_ID))
	{
		QSignalBlocker blocker(lineEdit);
		lineEdit->setText(QString::fromStdString(value.instructions[0].getOperand().str));
	}
}

void TypeSimplePropertyWidget::createEnumLayout(const types::QGlacierValue &value)
{
	if (value.views.empty() || value.views[0].getType()->getKind() != gamelib::TypeKind::ENUM)
	{
		assert(false && "Wrong view");
		return;
	}

	QStringList possibleValues;

	for (const auto& entry: reinterpret_cast<const gamelib::TypeEnum*>(value.views[0].getType())->getPossibleValues())
	{
		possibleValues.push_back(QString::fromStdString(entry.name));
	}

	auto layout = new QHBoxLayout(this);

	auto comboBox = new QComboBox(this);
	comboBox->setModel(new QStringListModel(possibleValues, comboBox));
	comboBox->setCurrentText(QString::fromStdString(value.instructions[0].getOperand().str));
	comboBox->setAccessibleName(ENUM_COMBOBOX_ID);
	comboBox->setEditable(false);
	connect(comboBox, &QComboBox::currentTextChanged, [this](const QString& newValue) {
		m_value.instructions[0] = PRPInstruction(m_value.instructions[0].getOpCode(), PRPOperandVal(newValue.toStdString()));
		valueChanged();
	});

	layout->addWidget(comboBox);
}

void TypeSimplePropertyWidget::updateEnumLayout(const types::QGlacierValue &value)
{
	if (auto comboBox = findChild<QComboBox*>(ENUM_COMBOBOX_ID))
	{
		QSignalBlocker blocker(comboBox);
		comboBox->setCurrentText(QString::fromStdString(value.instructions[0].getOperand().str));
	}
}

void TypeSimplePropertyWidget::paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &value)
{
	QTextOption textOptions;
	textOptions.setAlignment(Qt::AlignCenter);

	if (value.instructions[0].isBool())
	{
		painter->drawText(option.rect, QString("%1").arg(value.instructions[0].getOperand().trivial.b ? "YES": "NO"), textOptions);

#if 0 // NOTE: This part of code works well, but it looks not good enough
		painter->translate(option.rect.center());

		QStyleOptionButton checkbox;
		checkbox.state |= value.instructions[0].getOperand().trivial.b ? QStyle::State_On : QStyle::State_Off;
		checkbox.state |= QStyle::State_Enabled;

		QApplication::style()->drawControl(QStyle::ControlElement::CE_CheckBox, &checkbox, painter);
#endif
	}
	else if (value.instructions[0].isNumber())
	{
		QString text;

		switch (value.instructions[0].getOpCode())
		{
			case PRPOpCode::Int8:
			case PRPOpCode::NamedInt8:
			case PRPOpCode::Int16:
			case PRPOpCode::NamedInt16:
			case PRPOpCode::Int32:
			case PRPOpCode::NamedInt32:
			    text = QString("%1").arg(value.instructions[0].getOperand().trivial.i32);
				break;
		    case PRPOpCode::Float32:
		    case PRPOpCode::NamedFloat32:
			    text = QString("%1").arg(value.instructions[0].getOperand().trivial.f32);
			    break;
		    case PRPOpCode::Float64:
		    case PRPOpCode::NamedFloat64:
			    text = QString("%1").arg(value.instructions[0].getOperand().trivial.f64);
			    break;
		    default:
			    return;
		}

		painter->drawText(option.rect, text, textOptions);
	}
	else if (value.instructions[0].isString())
	{
		painter->drawText(option.rect, QString::fromStdString(value.instructions[0].getOperand().str), textOptions);
	}
	else if (value.instructions[0].isEnum())
	{
		QStyleOptionComboBox comboBox;
		comboBox.currentText = QString::fromStdString(value.instructions[0].getOperand().str);
		comboBox.editable = false;
		comboBox.state = option.state;
		comboBox.state |= QStyle::State_Enabled;
		comboBox.rect = option.rect;

		QApplication::style()->drawComplexControl(QStyle::ComplexControl::CC_ComboBox, &comboBox, painter);
		QApplication::style()->drawControl(QStyle::ControlElement::CE_ComboBoxLabel, &comboBox, painter);
	}
}