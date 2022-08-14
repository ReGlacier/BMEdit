#include <GameLib/TypeContainer.h>


namespace gamelib
{
	using namespace prp;

	TypeContainer::TypeContainer(std::string typeName)
		: Type(TypeKind::CONTAINER, std::move(typeName))
	{
	}

	Type::VerificationResult TypeContainer::verify(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!instructions || !instructions[0].hasValue())
		{
			return std::make_pair(false, nullptr);
		}

		if (instructions[0].getOpCode() != PRPOpCode::Container && instructions[0].getOpCode() != PRPOpCode::NamedContainer)
		{
			return std::make_pair(false, nullptr);
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		if (instructions.size() < capacity + 1)
		{
			return std::make_pair(false, nullptr);
		}

		return std::make_pair(true, instructions.slice(capacity + 1, instructions.size() - (capacity + 1)));
	}

	Type::DataMappingResult TypeContainer::map(const Span<PRPInstruction> &instructions) const
	{
		const auto& [verificationResult, _span] = verify(instructions);

		if (!verificationResult)
		{
			return {};
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		const int sliceSize = capacity + 1;
		std::vector<PRPInstruction> data;
		data.reserve(sliceSize);

		for (int i = 0; i < sliceSize; i++)
		{
			auto& ent = data.emplace_back();
			ent = instructions[i];
		}

		return Type::DataMappingResult(Value(this, std::move(data)), instructions.slice(sliceSize, instructions.size() - sliceSize));
	}
}