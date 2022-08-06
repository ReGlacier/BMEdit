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
		// Copy data instructions (in any case)
		std::copy(another.m_data.begin(), another.m_data.end(), std::back_inserter(m_data));

		// Copy views
		std::copy(another.m_views.begin(), another.m_views.end(), std::back_inserter(m_views));

		// Return self
		return *this;
	}

	Value &Value::operator+=(const std::pair<std::string, Value> &another)
	{
		const auto& [chunkName, chunkData] = another;

		// Save instructions pointer
		auto ip = m_data.size();

		// Copy instructions
		std::copy(chunkData.m_data.begin(), chunkData.m_data.end(), std::back_inserter(m_data));

		// Create entry
		auto& newEnt = m_entries.emplace_back();
		newEnt.name = chunkName;
		newEnt.instructions.iOffset = static_cast<int64_t>(ip);
		newEnt.instructions.iSize = static_cast<int64_t>(chunkData.m_data.size());

		// Copy views
		std::copy(chunkData.m_views.begin(), chunkData.m_views.end(), std::back_inserter(newEnt.views));
		std::copy(chunkData.m_views.begin(), chunkData.m_views.end(), std::back_inserter(m_views)); //TODO: Remove

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

	Span<ValueEntry> Value::getEntries() const
	{
		return Span<ValueEntry>(m_entries);
	}
}