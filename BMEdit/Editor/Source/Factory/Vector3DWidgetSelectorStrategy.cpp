#include <Factory/Vector3DWidgetSelectorStrategy.h>
#include <Widgets/TypePropertyWidget.h>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QObject>


using namespace factory;

using gamelib::prp::PRPOpCode;
using gamelib::prp::PRPInstruction;
using gamelib::prp::PRPOperandVal;


bool Vector3DWidgetSelectorStrategy::isAcceptableFor(const types::QGlacierValue & value)
{
	const auto& instructions = value.instructions;

	return
	    instructions.front().isBeginArray() &&
	    instructions.front().isSet() &&
	    instructions.front().getOperand().trivial.i32 == 3 &&
	    instructions.back().isEndArray();
}

void Vector3DWidgetSelectorStrategy::createAndPlace(const types::QGlacierValue & value, QWidget *owner)
{
	auto root = qobject_cast<widgets::TypePropertyWidget*>(owner);
	if (!root)
	{
		assert(false && "Expected TypePropertyWidget, got something else");
		return;
	}

	auto layout = new QHBoxLayout(owner);

	QWidget* x = nullptr;
	QWidget* y = nullptr;
	QWidget* z = nullptr;

	if (auto entType = value.instructions[1].getOpCode(); entType == PRPOpCode::Float32 || entType == PRPOpCode::NamedFloat32 || entType == PRPOpCode::Float64 || entType == PRPOpCode::NamedFloat64)
	{
		x = new QDoubleSpinBox(owner);
		y = new QDoubleSpinBox(owner);
		z = new QDoubleSpinBox(owner);

		auto setup = [](QDoubleSpinBox* w, const PRPInstruction& instruction)
		{
			if (!w)
			{
				assert(false);
				return;
			}

			const auto entType = instruction.getOpCode();

			if (entType == PRPOpCode::Float32 || entType == PRPOpCode::NamedFloat32)
			{
				w->setValue(instruction.getOperand().trivial.f32);
				w->setMinimum(std::numeric_limits<float>::lowest());
				w->setMaximum(std::numeric_limits<float>::max());
			}
			else if (entType == PRPOpCode::Float64 || entType == PRPOpCode::NamedFloat64)
			{
				w->setValue(instruction.getOperand().trivial.f64);
				w->setMinimum(std::numeric_limits<double>::lowest());
				w->setMaximum(std::numeric_limits<double>::max());
			}
		};

		setup(qobject_cast<QDoubleSpinBox*>(x), value.instructions[1]);
		setup(qobject_cast<QDoubleSpinBox*>(y), value.instructions[2]);
		setup(qobject_cast<QDoubleSpinBox*>(z), value.instructions[3]);

		auto routine = [root](double value, int idx) {
			auto prpValue = root->getValue();

			if (auto opCode = prpValue.instructions[idx].getOpCode(); opCode == PRPOpCode::NamedFloat32 || opCode == PRPOpCode::Float32)
			{
				prpValue.instructions[idx] = PRPInstruction(opCode, PRPOperandVal(static_cast<float>(value)));
			}
			else
			{
				prpValue.instructions[idx] = PRPInstruction(opCode, PRPOperandVal(value));
			}

			root->updateValue(prpValue);
		};

		QObject::connect(qobject_cast<QDoubleSpinBox*>(x), &QDoubleSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 1); });
		QObject::connect(qobject_cast<QDoubleSpinBox*>(y), &QDoubleSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 2); });
		QObject::connect(qobject_cast<QDoubleSpinBox*>(z), &QDoubleSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 3); });
	}
	else if (entType == PRPOpCode::Int8 || entType == PRPOpCode::NamedInt8 || entType == PRPOpCode::Int16 || entType == PRPOpCode::NamedInt16 || entType == PRPOpCode::Int32 || entType == PRPOpCode::NamedInt32)
	{
		x = new QSpinBox(owner);
		y = new QSpinBox(owner);
		z = new QSpinBox(owner);

		auto setup = [](QSpinBox* w, const PRPInstruction& instruction)
		{
			if (!w)
			{
				assert(false);
				return;
			}

			const auto entType = instruction.getOpCode();

			if (entType == PRPOpCode::Int8 || entType == PRPOpCode::NamedInt8)
			{
				w->setValue(instruction.getOperand().trivial.i8);
				w->setMinimum(std::numeric_limits<int8_t>::lowest());
				w->setMaximum(std::numeric_limits<int8_t>::max());
			}
			else if (entType == PRPOpCode::Int16 || entType == PRPOpCode::NamedInt16)
			{
				w->setValue(instruction.getOperand().trivial.i16);
				w->setMinimum(std::numeric_limits<int16_t>::lowest());
				w->setMaximum(std::numeric_limits<int16_t>::max());
			}
			else if (entType == PRPOpCode::Int32 || entType == PRPOpCode::NamedInt32)
			{
				w->setValue(instruction.getOperand().trivial.i32);
				w->setMinimum(std::numeric_limits<int32_t>::lowest());
				w->setMaximum(std::numeric_limits<int32_t>::max());
			}
		};

		setup(qobject_cast<QSpinBox*>(x), value.instructions[1]);
		setup(qobject_cast<QSpinBox*>(y), value.instructions[2]);
		setup(qobject_cast<QSpinBox*>(z), value.instructions[3]);

		auto routine = [root](int value, int idx) {
			auto prpValue = root->getValue();

			if (auto opCode = prpValue.instructions[idx].getOpCode(); opCode == PRPOpCode::Int32 || opCode == PRPOpCode::NamedInt32)
			{
				prpValue.instructions[idx] = PRPInstruction(opCode, PRPOperandVal(value));
			}
			else if (opCode == PRPOpCode::Int16 || opCode == PRPOpCode::NamedInt16)
			{
				prpValue.instructions[idx] = PRPInstruction(opCode, PRPOperandVal(static_cast<int16_t>(value)));
			}
			else if (opCode == PRPOpCode::Int8 || opCode == PRPOpCode::NamedInt8)
			{
				prpValue.instructions[idx] = PRPInstruction(opCode, PRPOperandVal(static_cast<int8_t>(value)));
			}

			root->updateValue(prpValue);
		};

		QObject::connect(qobject_cast<QSpinBox*>(x), &QSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 1); });
		QObject::connect(qobject_cast<QSpinBox*>(y), &QSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 2); });
		QObject::connect(qobject_cast<QSpinBox*>(z), &QSpinBox::valueChanged, [routine](auto && PH1) { return routine(std::forward<decltype(PH1)>(PH1), 3); });
	}

	if (x && y && z)
	{
		x->setAccessibleName("X");
		y->setAccessibleName("Y");
		z->setAccessibleName("Z");

		layout->addWidget(x);
		layout->addWidget(y);
		layout->addWidget(z);
	}
	else assert(false);

	owner->setLayout(layout);
}

Vector3DWidgetSelectorStrategy Vector3DWidgetSelectorStrategy::s_instance{};