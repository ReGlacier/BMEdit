#include <BMRender/BufferDescription.h>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <algorithm>


namespace bmr
{
	size_t getSizeOfType(DescriptionFieldType type)
	{
		switch (type)
		{
			case DescriptionFieldType::DFT_Bool:    return 1;
			case DescriptionFieldType::DFT_Int:
			case DescriptionFieldType::DFT_UInt:
			case DescriptionFieldType::DFT_Float:   return 4;
			case DescriptionFieldType::DFT_Vec2:    return sizeof(glm::vec2);
			case DescriptionFieldType::DFT_IVec2:   return sizeof(glm::ivec2);
			case DescriptionFieldType::DFT_Vec3:    return sizeof(glm::vec3);
			case DescriptionFieldType::DFT_IVec3:   return sizeof(glm::ivec3);
			case DescriptionFieldType::DFT_Vec4:    return sizeof(glm::vec4);
			case DescriptionFieldType::DFT_IVec4:   return sizeof(glm::ivec4);
			case DescriptionFieldType::DFT_Mat3x3:  return sizeof(glm::mat3);
			case DescriptionFieldType::DFT_Mat4x4:  return sizeof(glm::mat4);
		    default: return 0;
		}

		return 0;
	}

	BufferDescription::BufferDescription() = default;

	const std::vector<DescriptionField> &BufferDescription::getFields() const
	{
		updateFieldsOrder();

		return m_fields;
	}

	BufferDescription &BufferDescription::addField(int index, DescriptionFieldType type, bool isNormalized)
	{
		auto& field = m_fields.emplace_back();
		field.index = index;
		field.normalized = isNormalized;
		field.type = type;
		field.size = getSizeOfType(type);
		m_stride += field.size;
		m_isOrdered = false;

		return *this;
	}

	size_t BufferDescription::getStride() const
	{
		return m_stride;
	}

	void BufferDescription::forEach(const std::function<void(int index, DescriptionFieldType type, size_t size, bool isNormalized, int offset, int stride)> &pred) const
	{
		updateFieldsOrder();

		const int stride = static_cast<int>(getStride());
		int off = 0;

		for (const auto& it : getFields())
		{
			if (pred)
			{
				pred(it.index, it.type, it.size, it.normalized, off, stride);
			}

			off += static_cast<int>(it.size);
		}
	}

	void BufferDescription::updateFieldsOrder() const
	{
		if (!m_isOrdered)
		{
			m_isOrdered = true;

			std::sort(m_fields.begin(), m_fields.end(), [](const DescriptionField &first, const DescriptionField &second) {
				return first.index < second.index;
			});
		}
	}
}