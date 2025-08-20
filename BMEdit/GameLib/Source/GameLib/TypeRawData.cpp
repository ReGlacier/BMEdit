#include <GameLib/TypeRawData.h>


namespace gamelib
{
	using namespace prp;

	TypeRawData::TypeRawData(std::string typeName)
		: Type(TypeKind::RAW_DATA, std::move(typeName))
	{
	}

	Type::VerificationResult TypeRawData::verify(const Span<prp::PRPInstruction>& instructions) const
	{
		if (!instructions)
		{
			return std::make_pair(false, nullptr);
		}

		if (instructions[0].getOpCode() != PRPOpCode::Container && instructions[0].getOpCode() != PRPOpCode::NamedContainer)
		{
			return std::make_pair(false, nullptr);
		}

		if (!instructions[0].hasValue())
		{
			return std::make_pair(false, nullptr);
		}

		if (auto length = instructions[0].getOperand().trivial.i32; length > 0)
		{
			if (instructions[1].getOpCode() == PRPOpCode::RawData || instructions[1].getOpCode() == PRPOpCode::NamedRawData)
			{
				return std::make_pair(true, instructions.slice(2, instructions.size() - 2));
			}

			return std::make_pair(false, nullptr);
		}

		return std::make_pair(true, instructions.slice(1, instructions.size() - 1));
	}

	Type::DataMappingResult TypeRawData::map(const Span<prp::PRPInstruction> &instructions) const
	{
		const auto& [verificationResult, _span] = verify(instructions);

		if (!verificationResult)
		{
			return {};
		}

		if (instructions[0].getOperand().trivial.i32 == 0)
		{
			// Take only first instruction
			std::vector<PRPInstruction> data { instructions[0], PRPInstruction(PRPOpCode::RawData, PRPOperandVal(RawData {})) };

			return Type::DataMappingResult(Value(this, std::move(data)), instructions.slice(1, instructions.size() - 1));
		}

		// Take only one instruction
		std::vector<PRPInstruction> data { instructions[0], instructions[1] };
		return Type::DataMappingResult (Value(this, std::move(data)), instructions.slice(2, instructions.size() - 2));
	}

	Value TypeRawData::makeDefaultPropertiesPack() const
	{
		// Produce an empty Container opcode
		constexpr int32_t kEmptyContainerSize = 0;
		return Value(this, { PRPInstruction(PRPOpCode::Container, PRPOperandVal(kEmptyContainerSize)) });
	}
}