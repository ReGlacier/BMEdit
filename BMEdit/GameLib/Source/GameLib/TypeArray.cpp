#include <GameLib/TypeArray.h>


namespace gamelib
{
	using namespace prp;

	TypeArray::TypeArray(std::string typeName, PRPOpCode entryType, uint32_t requiredCapacity)
		: Type(TypeKind::ARRAY, std::move(typeName))
		, m_entryType(entryType)
		, m_requiredCapacity(requiredCapacity)
	{
	}

	TypeArray::TypeArray(std::string typeName,
	                     prp::PRPOpCode entryType,
	                     uint32_t requiredCapacity,
	                     std::vector<ValueView> &&valueViews)
         : Type(TypeKind::ARRAY, std::move(typeName))
         , m_entryType(entryType)
         , m_requiredCapacity(requiredCapacity)
         , m_valueViews(std::move(valueViews))
	{
	}

	PRPOpCode TypeArray::getTypeOfEntry() const
	{
		return m_entryType;
	}

	uint32_t TypeArray::getRequiredCapacity() const
	{
		return m_requiredCapacity;
	}

	const std::vector<ValueView> &TypeArray::getValueViews() const
	{
		return m_valueViews;
	}

	Span<prp::PRPInstruction> TypeArray::verifyInstructionSet(const Span<PRPInstruction> &instructions) const
	{
		if (!instructions || instructions.size < 2)
		{
			return {};
		}

		if (instructions[0].getOpCode() != PRPOpCode::Array && instructions[0].getOpCode() != PRPOpCode::NamedArray)
		{
			return {};
		}

		if (!instructions[0].hasValue())
		{
			return {};
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		if (capacity != m_requiredCapacity || (instructions.size < (2 + capacity))) // capacity must be same to inner size and we need to have 2 + capacity instructions in deck (2 - begin & end)
		{
			return {};
		}

		for (int i = 0; i < capacity; i++)
		{
			if (!instructions[1 + i].hasValue() || instructions[1 + i].getOpCode() != m_entryType)
			{
				return {};
			}
		}

		if (instructions[2 + capacity].getOpCode() != PRPOpCode::EndArray)
		{
			return {};
		}

		return instructions.slice((2 + capacity), instructions.size - (2 + capacity));
	}

	Type::DataMappingResult TypeArray::map(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!instructions)
		{
			return Type::DataMappingResult();
		}

		if (!verifyInstructionSet(instructions))
		{
			return Type::DataMappingResult();
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		const int sliceSize = 2 + capacity;

		std::vector<PRPInstruction> data;
		data.reserve(sliceSize);

		for (int i = 0; i < sliceSize; i++)
		{
			auto& ent = data.emplace_back();
			ent = instructions[i];
		}

		return Type::DataMappingResult(
			Value(this, std::move(data), m_valueViews),
			instructions.slice(sliceSize, instructions.size - sliceSize)
		);
	}
}