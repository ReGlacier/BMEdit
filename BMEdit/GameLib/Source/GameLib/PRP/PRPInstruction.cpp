#include  <GameLib/PRP/PRPInstruction.h>

#include <utility>


namespace gamelib::prp
{
	PRPInstruction::PRPInstruction(PRPOpCode opCode)
		: m_opCode(opCode)
	{
		updateFlags();
	}

	PRPInstruction::PRPInstruction(PRPOpCode opCode, PRPOperandVal operand)
		: m_opCode(opCode)
		, m_isSet(true)
		, m_operand(std::move(operand))
	{
		updateFlags();
	}

	bool PRPInstruction::isSet() const
	{
		return m_isSet;
	}

	bool PRPInstruction::isNamed() const
	{
		return m_isNamed;
	}

	bool PRPInstruction::isDeclarator() const
	{
		return m_isDeclarator;
	}

	bool PRPInstruction::isBeginObject() const
	{
		return (m_opCode == PRPOpCode::BeginObject) || (m_opCode == PRPOpCode::BeginNamedObject);
	}

	bool PRPInstruction::isBeginArray() const
	{
		if (!isSet())
		{
			return false;
		}

		return (m_opCode == PRPOpCode::Array) || (m_opCode == PRPOpCode::NamedArray);
	}

	bool PRPInstruction::isEndObject() const
	{
		return m_opCode == PRPOpCode::EndObject;
	}

	bool PRPInstruction::isEndArray() const
	{
		return m_opCode == PRPOpCode::EndArray;
	}

	bool PRPInstruction::isEndOfStream() const
	{
		return m_opCode == PRPOpCode::EndOfStream;
	}

	bool PRPInstruction::isTrivialValue() const
	{
		if (!hasValue())
		{
			return false;
		}

		return isString() || isNumber() || isBool();
	}

	bool PRPInstruction::isBool() const
	{
		if (!hasValue())
		{
			return false;
		}

		return (m_opCode == PRPOpCode::Bool) || (m_opCode == PRPOpCode::NamedBool);
	}

	bool PRPInstruction::isString() const
	{
		if (!hasValue())
		{
			return false;
		}

		return (m_opCode == PRPOpCode::String) || (m_opCode == PRPOpCode::NamedString);
	}

	bool PRPInstruction::isNumber() const
	{
		if (!hasValue())
		{
			return false;
		}

		return
			(m_opCode == PRPOpCode::Int8) || (m_opCode == PRPOpCode::Int16) || (m_opCode == PRPOpCode::Int32) ||
			(m_opCode == PRPOpCode::NamedInt8) || (m_opCode == PRPOpCode::NamedInt16) || (m_opCode == PRPOpCode::NamedInt32) ||
			(m_opCode == PRPOpCode::Float32) || (m_opCode == PRPOpCode::Float64) ||
			(m_opCode == PRPOpCode::NamedFloat32) || (m_opCode == PRPOpCode::NamedFloat64) || (m_opCode == PRPOpCode::Bitfield);
	}

	bool PRPInstruction::isEnum() const
	{
		if (!hasValue())
		{
			return false;
		}

		return (m_opCode == PRPOpCode::StringOrArray_E) || (m_opCode == PRPOpCode::StringOrArray_8E);
	}

	bool PRPInstruction::hasValue() const
	{
		return m_isSet && !m_isDeclarator;
	}

	PRPOpCode PRPInstruction::getOpCode() const
	{
		return m_opCode;
	}

	const PRPOperandVal &PRPInstruction::getOperand() const
	{
		return m_operand;
	}

	void PRPInstruction::updateFlags()
	{
		updateIsDeclaratorFlag();
		updateIsNamedFlag();
	}

	void PRPInstruction::updateIsNamedFlag()
	{
		m_isNamed = false; // reset flag
		m_isNamed |= (m_opCode == PRPOpCode::NamedArray);
		m_isNamed |= (m_opCode == PRPOpCode::BeginNamedObject);
		m_isNamed |= (m_opCode == PRPOpCode::NamedReference);
		m_isNamed |= (m_opCode == PRPOpCode::NamedContainer);
		m_isNamed |= (m_opCode == PRPOpCode::NamedChar);
		m_isNamed |= (m_opCode == PRPOpCode::NamedBool);
		m_isNamed |= (m_opCode == PRPOpCode::NamedInt8);
		m_isNamed |= (m_opCode == PRPOpCode::NamedInt16);
		m_isNamed |= (m_opCode == PRPOpCode::NamedInt32);
		m_isNamed |= (m_opCode == PRPOpCode::NamedFloat32);
		m_isNamed |= (m_opCode == PRPOpCode::NamedFloat64);
		m_isNamed |= (m_opCode == PRPOpCode::NamedString);
		m_isNamed |= (m_opCode == PRPOpCode::NamedRawData);
	}

	void PRPInstruction::updateIsDeclaratorFlag()
	{
		m_isDeclarator = false;
		m_isDeclarator |= (m_opCode == PRPOpCode::BeginObject);
		m_isDeclarator |= (m_opCode == PRPOpCode::BeginNamedObject);
		m_isDeclarator |= (m_opCode == PRPOpCode::EndArray);
		m_isDeclarator |= (m_opCode == PRPOpCode::EndOfStream);
		m_isDeclarator |= (m_opCode == PRPOpCode::EndObject);
		m_isDeclarator |= (m_opCode == PRPOpCode::SkipMark);
	}
}