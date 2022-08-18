#include <Editor/PropertyAsStringRepr.h>

using namespace editor;
using gamelib::prp::PRPOpCode;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPInstruction;


QVariant PropertyAsStringRepr::getStringRepresentationOfValue(const types::QGlacierValue& value)
{
	/**
	 * TODO: Wrap those thing to strategies
	 */
	if (value.instructions.size() == 5 && value.instructions.front().isBeginArray() && value.instructions.back().isEndArray())
	{
		if (const auto& entType = value.instructions[1].getOpCode(); entType == PRPOpCode::Float32 || entType == PRPOpCode::NamedFloat32)
		{
			return QString("(%1;%2;%3)")
			    .arg(value.instructions[1].getOperand().trivial.f32)
			    .arg( value.instructions[2].getOperand().trivial.f32)
			    .arg( value.instructions[3].getOperand().trivial.f32);
		}
		else if (entType == PRPOpCode::Int8 || entType == PRPOpCode::NamedInt8)
		{
			return QString("(%1;%2;%3)")
			    .arg(value.instructions[1].getOperand().trivial.i8)
			    .arg(value.instructions[2].getOperand().trivial.i8)
			    .arg(value.instructions[3].getOperand().trivial.i8);
		}
		else if (entType == PRPOpCode::Int16 || entType == PRPOpCode::NamedInt16)
		{
			return QString("(%1;%2;%3)")
			    .arg(value.instructions[1].getOperand().trivial.i16)
			    .arg(value.instructions[2].getOperand().trivial.i16)
			    .arg(value.instructions[3].getOperand().trivial.i16);
		}
		else if (entType == PRPOpCode::Int32 || entType == PRPOpCode::NamedInt32)
		{
			return QString("(%1;%2;%3)")
			    .arg(value.instructions[1].getOperand().trivial.i32)
			    .arg(value.instructions[2].getOperand().trivial.i32)
			    .arg(value.instructions[3].getOperand().trivial.i32);
		}
	}

	return {};
}