#pragma once

#include <GameLib/MAT/MATEntries.h>
#include <cstdint>
#include <variant>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATValU
	{
	public:
		using Value = std::variant<float, uint32_t>;

		MATValU();
		explicit MATValU(std::vector<Value>&& values);

		MATValU& operator=(std::vector<Value>&& values) noexcept;

		static MATValU makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, const MATPropertyEntry& selfDecl);

		[[nodiscard]] const std::vector<Value>& getValues() const { return m_values; }

	private:
		std::vector<Value> m_values {};
	};
}