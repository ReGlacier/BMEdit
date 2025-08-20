//
// Code from this file is based on BestByte Framework created by DronCode
//
#pragma once

#include <functional>
#include <cstdint>
#include <vector>


namespace render
{
	/**
	 * @note If you would like to change this you need to add changes in g_typeSizeMap and g_entryTypeToGLType arrays.
	 */
	enum class VertexDescriptionEntryType
	{
		VDE_None = 0,
		VDE_Bool,
		VDE_Int32,
		VDE_UInt32,
		VDE_Float32,
		VDE_Vec2,
		VDE_Vec3,
		VDE_Vec4,
		VDE_IVec2,
		VDE_IVec3,
		VDE_IVec4,
		VDE_Mat3x3,
		VDE_Mat4x4
	};

	struct VertexDescriptionEntry
	{
		uint32_t index { 0 };
		uint32_t offset { 0 };
		uint32_t size { 0 };
		VertexDescriptionEntryType type { VertexDescriptionEntryType::VDE_None };
		bool normalized { false };
	};

	class VertexFormatDescription
	{
	public:
		using Entries = std::vector<VertexDescriptionEntry>;
		using Visitor = std::function<void(
		                    uint32_t, // index
		                    uint32_t, // offset
		                    uint32_t, // size
		                    uint32_t, // stride
		                    VertexDescriptionEntryType, // type
		                    bool // is normalized
		)>;

		VertexFormatDescription();

		VertexFormatDescription& addField(uint32_t index, VertexDescriptionEntryType type, bool normalized = false);

		[[nodiscard]] const Entries& getEntries() const;
		void visit(const Visitor& visitor) const;

		[[nodiscard]] uint32_t getStride() const;

	private:
		void updateOrder() const;

	private:
		mutable Entries m_entries {};
		uint32_t m_stride { 0 };
		mutable bool m_isOrdered { false };
	};
}