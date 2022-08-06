#include <GameLib/TypeEnum.h>
#include <stdexcept>


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

		//NOTE: In some implementations enum value must be represented as operand.trivial.i32 but we ignoring that here
		for (const auto& [name, _value]: m_possibleValues)
		{
			if (name == operand.str)
			{
				return std::make_pair(true, instructions.slice(1, instructions.size - 1));
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

	const TypeEnum::Entries &TypeEnum::getPossibleValues() const
	{
		return m_possibleValues;
	}
}