#include <GameLib/TypeBitfield.h>


namespace gamelib
{
	using namespace prp;

	TypeBitfield::TypeBitfield(std::string typeName, TypeBitfield::PossibleOptions &&possibleOptions)
		: Type(TypeKind::BITFIELD, std::move(typeName))
		, m_possibleOptions(std::move(possibleOptions))
	{
	}

	const TypeBitfield::PossibleOptions &TypeBitfield::getPossibleOptions() const
	{
		return m_possibleOptions;
	}

	Span<prp::PRPInstruction> TypeBitfield::verifyInstructionSet(const Span<PRPInstruction> &instructions) const
	{
		if (!instructions)
		{
			return {};
		}

		if (instructions[0].getOpCode() != PRPOpCode::StringArray)
		{
			return {};
		}

		for (const auto &entry: instructions[0].getOperand().stringArray)
		{
			if (!m_possibleOptions.contains(entry))
			{
				return {};
			}
		}

		return instructions.slice(1, instructions.size - 1);
	}

	Type::DataMappingResult TypeBitfield::map(const Span<PRPInstruction> &instructions) const
	{
		if (!verifyInstructionSet(instructions))
		{
			return Type::DataMappingResult();
		}

		std::vector<PRPInstruction> data { instructions[0] };
		return Type::DataMappingResult(Value(this, std::move(data)), instructions.slice(1, instructions.size - 1));
	}
}