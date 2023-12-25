#include <GameLib/TypeArray.h>
#include <sstream>


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

	Type::VerificationResult TypeArray::verify(const Span<prp::PRPInstruction>& instructions) const
	{
		if (!instructions || instructions.size() < 2)
		{
			return std::make_pair(false, nullptr);
		}

		if (instructions[0].getOpCode() != PRPOpCode::Array && instructions[0].getOpCode() != PRPOpCode::NamedArray)
		{
			return std::make_pair(false, nullptr);
		}

		if (!instructions[0].hasValue())
		{
			return std::make_pair(false, nullptr);
		}

		const int capacity = instructions[0].getOperand().trivial.i32;
		if (capacity != m_requiredCapacity || (instructions.size() < (2 + capacity))) // capacity must be same to inner size, and we need to have 2 + capacity instructions in deck (2 - begin & end)
		{
			return std::make_pair(false, nullptr);
		}

		for (int i = 0; i < capacity; i++)
		{
			if (!instructions[1 + i].hasValue() || instructions[1 + i].getOpCode() != m_entryType)
			{
				return std::make_pair(false, nullptr);
			}
		}

		if (instructions[1 + capacity].getOpCode() != PRPOpCode::EndArray)
		{
			return std::make_pair(false, nullptr);
		}

		return std::make_pair(true, instructions.slice((2 + capacity), instructions.size() - (2 + capacity)));
	}

	Type::DataMappingResult TypeArray::map(const Span<prp::PRPInstruction> &instructions) const
	{
		if (!instructions)
		{
			return {};
		}

		const auto& [verificationResult, _nextSpan] = verify(instructions);

		if (!verificationResult)
		{
			return {};
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

		std::vector<ValueView> views = m_valueViews;

		if (views.empty())
		{
			// Here we need to generate value views
			views.resize(m_requiredCapacity);

			for (auto i = 0; i < m_requiredCapacity; i++)
			{
				std::stringstream ss;
				ss << "[" << i << "]";

				views[i] = ValueView(ss.str(), m_entryType, this);
			}
		}

		return std::make_pair(
		    Value(this, std::move(data), views),
		    instructions.slice(sliceSize, instructions.size() - sliceSize));
	}

	Value TypeArray::makeDefaultPropertiesPack() const
	{
		// create an array. It presented via at least 3 instructions: BeginArray, [m_entryType opcode], EndArray
		std::vector<PRPInstruction> instructions{};
		instructions.resize(2 + m_requiredCapacity);

		instructions.front() = PRPInstruction(PRPOpCode::Array, PRPOperandVal(static_cast<int32_t>(m_requiredCapacity)));
		instructions.back() = PRPInstruction(PRPOpCode::EndArray);

		// Fill by empty instructions. For float - 0.f, for int - 0, for bool - false, for string - ""
		for (int i = 1; i < m_requiredCapacity + 1; i++)
		{
			instructions[i] = PRPInstruction(m_entryType, prp::PRPOperandVal::kInitedOperandValue);
		}

		return Value(this, instructions);
	}
}