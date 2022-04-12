#include <GameLib/TypeContainer.h>


namespace gamelib
{
	using namespace prp;

	TypeContainer::TypeContainer(std::string typeName)
		: Type(TypeKind::CONTAINER, std::move(typeName))
	{
	}

	Span<PRPInstruction> TypeContainer::verifyInstructionSet(const Span<PRPInstruction> &instructions) const
	{
		if (!instructions || !instructions[0].hasValue())
		{
			return {};
		}

		if (instructions[0].getOpCode() != PRPOpCode::Container && instructions[0].getOpCode() != PRPOpCode::NamedContainer)
		{
			return {};
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		if (instructions.size < capacity + 1)
		{
			return {};
		}

		return instructions.slice(capacity + 1, instructions.size - (capacity + 1));
	}

	Type::DataMappingResult TypeContainer::map(const Span<PRPInstruction> &instructions) const
	{
		if (!verifyInstructionSet(instructions))
		{
			return Type::DataMappingResult();
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

		return Type::DataMappingResult(Value(this, std::move(data)), instructions.slice(sliceSize, instructions.size - sliceSize));
	}
}