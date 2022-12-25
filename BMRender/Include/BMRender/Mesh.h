#pragma once

#include <variant>
#include <cstdint>


namespace bmr
{
	enum class MeshFormat
	{
		MF_UNKNOWN = 0,
		MF_FORMAT_10 = 0x10,
		MF_FORMAT_24 = 0x24,
		MF_FORMAT_28 = 0x28,
		MF_FORMAT_34 = 0x34,
	};

	enum class MeshTopology
	{
		MT_TRIANGLES,
		MT_TRIANGLE_STRIP,
		MT_TRIANGLE_FAN,
		MT_LINES,
		MT_LINE_STRIP
	};

	struct BufferView
	{
		const void* data { nullptr };
		size_t size { 0 };

		BufferView& operator>>=(std::size_t offset)
		{
			if (size > offset && data)
			{
				size -= offset;
				data = reinterpret_cast<const void*>(reinterpret_cast<const unsigned char*>(data) + offset);
			}

			return *this;
		}

		template <typename T> BufferView& operator>>(T& result)
		{
			if (sizeof(T) < size && data)
			{
				result = *reinterpret_cast<const T*>(data);
				size -= sizeof(T);
				data = reinterpret_cast<const void*>(reinterpret_cast<const unsigned char*>(data) + sizeof(T));
			}

			return *this;
		}
	};

	namespace impl
	{
		using ResourceID = std::uint32_t;

		constexpr ResourceID kInvalidIdx { 0xBAD0B4BE };

		struct NullMeshData
		{
			void enable() {}
			void disable() {}
			void draw(MeshTopology) {}
		};

		struct SimpleMeshData
		{
			ResourceID vao { kInvalidIdx };
			ResourceID vbo { kInvalidIdx };
			ResourceID verticesNr { 0 };

			SimpleMeshData();
			~SimpleMeshData();

			void setup(MeshFormat format, BufferView vertices, int verticesCount);

			void enable();
			void disable();
			void draw(MeshTopology topology);
		};

		struct IndexedMeshData
		{
			ResourceID vao { kInvalidIdx };
			ResourceID vbo { kInvalidIdx };
			ResourceID ebo { kInvalidIdx };
			int indicesNr { 0 };
			int verticesNr { 0 };

			IndexedMeshData();
			~IndexedMeshData();

			void setup(MeshFormat format, BufferView vertices, BufferView indices, int indicesCount, int verticesCount);

			void enable();
			void disable();
			void draw(MeshTopology topology);
		};

		using MeshData = std::variant<NullMeshData, SimpleMeshData, IndexedMeshData>;
	}

	class Mesh
	{
	public:
		Mesh();

		void setMeshData(MeshFormat format, BufferView vertexBuffer, int verticesCount);
		void setMeshData(MeshFormat format, BufferView vertexBuffer, BufferView indexBuffer, int incidesCount, int verticesCount);

		void setTopology(MeshTopology topology);
		[[nodiscard]] MeshTopology getTopology() const;

		void draw();

	private:
		bool m_inUse { false };
		bool m_hasData { false };

		impl::MeshData m_data {};
		MeshTopology m_topology { MeshTopology::MT_TRIANGLES };
	};
}