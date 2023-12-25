#pragma once

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

#include <GameLib/MAT/MATEntries.h>
#include <GameLib/MAT/MATInstance.h>
#include <GameLib/MAT/MATClass.h>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATReader
	{
	public:
		MATReader();

		bool parse(const uint8_t* pMatBuffer, size_t iMatBufferSize);

		[[nodiscard]] const MATHeader& getHeader() const { return m_header; }
		[[nodiscard]] std::vector<MATClass>&& takeClasses() { return std::move(m_classes); }
		[[nodiscard]] std::vector<MATInstance>&& takeInstances() { return std::move(m_instances); }

	private:
		bool collectMaterialClasses(ZBio::ZBinaryReader::BinaryReader* matReader);
		bool collectMaterialInstances(ZBio::ZBinaryReader::BinaryReader* matReader);

	private:
		MATHeader m_header;
		std::vector<uint32_t>    m_constantTable;
		std::vector<MATClass>    m_classes;
		std::vector<MATInstance> m_instances;
	};
}