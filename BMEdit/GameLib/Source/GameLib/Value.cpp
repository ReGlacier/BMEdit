#include <GameLib/Value.h>
#include <GameLib/Type.h>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iterator>


namespace gamelib
{
	Value::Value() = default;

	Value::Value(const Type *type, std::vector<prp::PRPInstruction> data)
		: m_type(type), m_data(std::move(data))
	{
	}

	Value::Value(const Type *type, std::vector<prp::PRPInstruction> data, std::vector<ValueView> views)
		: m_type(type), m_data(std::move(data)), m_views(std::move(views))
	{
	}

	Value &Value::operator+=(const Value &another)
	{
		// Copy data instructions
		std::copy(another.m_data.begin(), another.m_data.end(), std::back_inserter(m_data));

		// Copy views
		std::copy(another.m_views.begin(), another.m_views.end(), std::back_inserter(m_views));

		// Return self
		return *this;
	}

	const Type *Value::getType() const
	{
		return m_type;
	}

	const std::vector<prp::PRPInstruction> &Value::getInstructions() const
	{
		return m_data;
	}

	int Value::getInstructionsCount() const
	{
		return static_cast<int>(m_data.size());
	}

	const std::vector<ValueView> &Value::getViews() const
	{
		return m_views;
	}

	const ValueView& Value::getView(int instructionIndex) const
	{
		static const ValueView kNullView { "", nullptr, nullptr };

		if (instructionIndex >= 0 && instructionIndex < m_views.size())
		{
			return m_views[instructionIndex];
		}

		return kNullView;
	}

	bool Value::hasView(int instructionIndex) const
	{
		if (instructionIndex < 0 || instructionIndex >= m_views.size()) {
			return false;
		}

		return true;
	}

	bool Value::setInstruction(int instructionIndex, const prp::PRPInstruction &instruction)
	{
		if (instructionIndex < 0 || instructionIndex >= m_data.size())
		{
			return false;
		}

		if (!prp::areOpCodesHasSameKind(m_data[instructionIndex].getOpCode(), instruction.getOpCode()))
		{
			// Instructions must be of the same kind
			return false;
		}

		m_data[instructionIndex] = instruction;
		return true;
	}

	bool Value::getInstruction(int instructionIndex, prp::PRPInstruction &instruction)
	{
		if (instructionIndex < 0 || instructionIndex >= m_data.size())
		{
			return false;
		}

		instruction = m_data[instructionIndex];
		return true;
	}
}