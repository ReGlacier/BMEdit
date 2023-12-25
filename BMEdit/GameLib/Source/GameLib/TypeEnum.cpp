#include <GameLib/TypeEnum.h>
#include <stdexcept>
#include <algorithm>


namespace gamelib
{
	using namespace prp;

	TypeEnum::TypeEnum(std::string typeName, Entries possibleValues)
		: Type(TypeKind::ENUM, std::move(typeName))
		, m_possibleValues(std::move(possibleValues))
	{
	}

	Type::VerificationResult TypeEnum::verify(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!instructions) {
			return std::make_pair(false, nullptr);
		}

		const auto& decl = instructions[0];
		if (!decl.hasValue()) {
			return std::make_pair(false, nullptr);; // unset value is invalid
		}

		const auto& opCode = decl.getOpCode();
		const auto& operand = decl.getOperand();

		if (opCode != PRPOpCode::StringOrArray_E && opCode != PRPOpCode::StringOrArray_8E)
		{
			// Operand must be StringOrArray_E or StringOrArray_8E
			return std::make_pair(false, nullptr);;
		}

		//NOTE: In some implementations enum value must be represented as operand.trivial.i32, but we're ignoring that here
		for (const auto& [name, _value]: m_possibleValues)
		{
			if (name == operand.str)
			{
				return std::make_pair(true, instructions.slice(1, instructions.size() - 1));
			}
		}

		return std::make_pair(false, nullptr);
	}

	Type::DataMappingResult TypeEnum::map(const Span<prp::PRPInstruction> &instructions) const
	{
		const auto& [verificationResult, newSlice] = verify(instructions);

		if (!verificationResult)
		{
			return Type::DataMappingResult(std::nullopt, Span<prp::PRPInstruction>());
		}

		std::vector<prp::PRPInstruction> valueData { instructions[0] };
		return Type::DataMappingResult(Value(this, std::move(valueData), { ValueView("Value", instructions[0].getOpCode(), this) }), newSlice);
	}

	Value TypeEnum::makeDefaultPropertiesPack() const
	{
		// Here we need to select lowest possible value of presented and return it. When we have an empty array we should raise an exception because our behaviour is undefined in this case
		if (m_possibleValues.empty())
			throw std::runtime_error("Invalid enum declaration! Unable to produce 'default' enum without any variations!");

		if (m_possibleValues.size() == 1)
		{
			// just return this entry anyway
			return Value(this, { PRPInstruction(PRPOpCode::StringOrArray_E, PRPOperandVal(m_possibleValues[0].name)) });
		}

		auto lowest = std::min_element(m_possibleValues.begin(), m_possibleValues.end(), [](const Entry& a, const Entry& b) { return a.value < b.value; });
		return Value(this, { PRPInstruction(PRPOpCode::StringOrArray_E, PRPOperandVal(lowest->name)) });
	}

	const TypeEnum::Entries &TypeEnum::getPossibleValues() const
	{
		return m_possibleValues;
	}
}