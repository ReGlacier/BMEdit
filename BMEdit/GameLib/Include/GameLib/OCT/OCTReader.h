#pragma once

#include <GameLib/OCT/OCTEntries.h>
#include <cstdint>
#include <vector>


namespace gamelib::oct
{
	class OCTReader
	{
	public:
		OCTReader();

		bool parse(const uint8_t* pOCTBuffer, size_t iBufferSize);

		[[nodiscard]] const OCTHeader& getHeader() const { return m_header; }
		[[nodiscard]] std::vector<OCTNode>&& takeNodes() { return std::move(m_nodes); }
		[[nodiscard]] std::vector<OCTObject>&& takeObjects() { return std::move(m_objects); }
		[[nodiscard]] std::vector<OCTUnknownBlock>&& takeUBS() { return std::move(m_unknownBlocks); }

	private:
		OCTHeader m_header {};
		std::vector<OCTNode> m_nodes {};
		std::vector<OCTObject> m_objects {};
		std::vector<OCTUnknownBlock> m_unknownBlocks {}; // associated with m_objects
	};
}