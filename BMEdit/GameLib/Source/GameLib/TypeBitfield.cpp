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

	Type::VerificationResult TypeBitfield::verify(const Span<prp::PRPInstruction>& instructions) const
	{
		if (!instructions)
		{
			return std::make_pair(false, nullptr);
		}

		if (instructions[0].getOpCode() != PRPOpCode::StringArray)
		{
			return std::make_pair(false, nullptr);
		}

		for (const auto &entry: instructions[0].getOperand().stringArray)
		{
			if (!m_possibleOptions.contains(entry))
			{
				return std::make_pair(false, nullptr);
			}
		}

		return std::make_pair(true, instructions.slice(1, instructions.size - 1));
	}

	Type::DataMappingResult TypeBitfield::map(const Span<PRPInstruction> &instructions) const
	{
		const auto& [verificationResult, span] = verify(instructions);

		if (!verificationResult)
		{
			return Type::DataMappingResult();
		}

		std::vector<PRPInstruction> data { instructions[0] };
		return Type::DataMappingResult(Value(this, std::move(data)), span);
	}
}