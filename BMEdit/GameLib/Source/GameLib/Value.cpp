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

	bool Value::operator==(const gamelib::Value &other) const
	{
		return !operator!=(other);
	}

	bool Value::operator!=(const gamelib::Value &other) const
	{
		if (this == &other)
			return false;

		if (m_type != other.m_type)
			return true;

		if (m_data != other.m_data)
			return true;

		if (m_entries != other.m_entries)
			return true;

		if (m_views != other.m_views)
			return true;

		return false;
	}

	const Type *Value::getType() const
	{
		return m_type;
	}

	const std::vector<prp::PRPInstruction> &Value::getInstructions() const
	{
		return m_data;
	}

	std::vector<prp::PRPInstruction> &Value::getInstructions()
	{
		return m_data;
	}

	Span<ValueEntry> Value::getEntries() const
	{
		return Span<ValueEntry>(m_entries);
	}

	void Value::updateContainer(int entryIndex, const std::vector<prp::PRPInstruction> &newDecl)
	{
		// So, let's update data chunk and then rebuild views
		const auto& [off, sz] = m_entries.at(entryIndex).instructions;
		m_data.erase(m_data.begin() + off, m_data.begin() + off + sz);
		std::copy(newDecl.begin(), newDecl.end(), std::inserter(m_data, m_data.begin() + off));

		// And then rebuild everything
		auto [mappedData, _newSlice] = m_type->map(Span(m_data));

		if (!mappedData.has_value())
		{
			throw std::runtime_error("Unable to map new data bunch. Looks like an internal error");
			return;
		}

		const auto& data = mappedData.value();
		m_data = data.m_data;
		m_entries = data.m_entries;
		m_views = data.m_views;
	}
}