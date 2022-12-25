#pragma once

#include <functional>
#include <vector>


namespace bmr
{
	enum class DescriptionFieldType
	{
		DFT_Unknown = 0,
		DFT_Bool,
		DFT_Int,
		DFT_UInt,
		DFT_Float,
		DFT_Vec2,
		DFT_Vec3,
		DFT_Vec4,
		DFT_IVec2,
		DFT_IVec3,
		DFT_IVec4,
		DFT_Mat3x3,
		DFT_Mat4x4,
	};

	struct DescriptionField
	{
		int                     index;
		DescriptionFieldType    type;
		size_t                  size;
		bool                    normalized;
	};

	class BufferDescription
	{
	public:
		BufferDescription();

		BufferDescription& addField(int index, DescriptionFieldType type, bool isNormalized = true);

		[[nodiscard]] const std::vector<DescriptionField>& getFields() const;
		[[nodiscard]] size_t getStride() const;
		void forEach(const std::function<void(int index, DescriptionFieldType type, size_t size, bool isNormalized, int offset, int stride)> &pred) const;

	private:
		void updateFieldsOrder() const;

	private:
		mutable std::vector<DescriptionField> m_fields {};
		mutable size_t m_stride{ 0 };
		mutable bool m_isOrdered{ false };
	};
}