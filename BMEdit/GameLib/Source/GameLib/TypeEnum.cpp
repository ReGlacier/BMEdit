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

	Span<prp::PRPInstruction> TypeEnum::verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!instructions) {
			return {};
		}

		const auto& decl = instructions[0];
		if (!decl.hasValue()) {
			return {}; // unset value is invalid
		}

		const auto& opCode = decl.getOpCode();
		const auto& operand = decl.getOperand();

		if (opCode != PRPOpCode::StringOrArray_E && opCode != PRPOpCode::StringOrArray_8E)
		{
			// Operand must be StringOrArray_E or StringOrArray_8E
			return {};
		}

		//NOTE: In some implementations enum value must be represented as operand.trivial.i32 but we ignoring that here
		for (const auto& [name, _value]: m_possibleValues)
		{
			if (name == operand.str)
			{
				return instructions.slice(1, instructions.size - 1);
			}
		}

		return {};
	}

	Type::DataMappingResult TypeEnum::map(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!verifyInstructionSet(instructions.slice(0, 1)))
		{
			return Type::DataMappingResult(std::nullopt, Span<prp::PRPInstruction>());
		}

		std::vector<prp::PRPInstruction> valueData { instructions[0] };
		return Type::DataMappingResult(Value(this, std::move(valueData)), instructions.slice(1, instructions.size - 1));
	}
}