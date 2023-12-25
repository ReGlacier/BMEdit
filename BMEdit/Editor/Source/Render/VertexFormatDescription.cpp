//
// Code from this file is based on BestByte Framework created by DronCode
//
#include <Render/VertexFormatDescription.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <algorithm>
#include <cassert>


namespace render
{
	static constexpr uint32_t g_typeSizeMap[] = {
	    0,  // None
	    sizeof(bool),
	    sizeof(int32_t),
	    sizeof(uint32_t),
	    sizeof(float),
	    sizeof(glm::vec2),
	    sizeof(glm::vec3),
	    sizeof(glm::vec4),
	    sizeof(glm::ivec2),
	    sizeof(glm::ivec3),
	    sizeof(glm::ivec4),
	    sizeof(glm::mat3x3),
	    sizeof(glm::mat4x4)
	};

	VertexFormatDescription::VertexFormatDescription() = default;

	VertexFormatDescription& VertexFormatDescription::addField(uint32_t index, VertexDescriptionEntryType type, bool normalized)
	{
		if (type == VertexDescriptionEntryType::VDE_None)
		{
			assert(false);
			return *this;
		}

		auto& ent = m_entries.emplace_back();
		ent.type = type;
		ent.size = g_typeSizeMap[static_cast<int>(type)];
		ent.normalized = normalized;
		ent.index = index;

		m_isOrdered = false;
		m_stride += ent.size;

		return *this;
	}

	const VertexFormatDescription::Entries& VertexFormatDescription::getEntries() const
	{
		updateOrder();

		return m_entries;
	}

	void VertexFormatDescription::visit(const render::VertexFormatDescription::Visitor &visitor) const
	{
		updateOrder();

		for (const auto& [index, offset, size, type, normalized] : m_entries)
		{
			visitor(index, offset, size, m_stride, type, normalized);
		}
	}

	uint32_t VertexFormatDescription::getStride() const
	{
		return m_stride;
	}

	void VertexFormatDescription::updateOrder() const
	{
		if (!m_isOrdered)
		{
			m_isOrdered = true;

			std::sort(m_entries.begin(), m_entries.end(), [](const VertexDescriptionEntry& a, const VertexDescriptionEntry& b) {
				return a.index < b.index;
			});

			size_t offset = 0;
			for (auto& entry : m_entries)
			{
				entry.offset = offset;
				offset += entry.size;
			}
		}
	}
}