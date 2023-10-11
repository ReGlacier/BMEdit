#include <GameLib/OCT/OCTReader.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib::oct
{
	OCTReader::OCTReader() = default;

	bool OCTReader::parse(const uint8_t* pOCTBuffer, size_t iBufferSize)
	{
		if (!pOCTBuffer || !iBufferSize)
			return false;

		ZBio::ZBinaryReader::BinaryReader octReader { reinterpret_cast<const char*>(pOCTBuffer), static_cast<int64_t>(iBufferSize) };

		// Read header
		OCTHeader::deserialize(m_header, &octReader);

		// Seek by 0x14 (after header)
		octReader.seek(0x14);

		// And read objects
		const uint32_t nodesCount = (m_header.objectsOffset - 0x14) / 0x6;  // 0x14  - always start point of nodes list, 0x6 - size of each node entry
		int totalObjects = 0;

		m_nodes.reserve(nodesCount);
		for (int i = 0; i < nodesCount; i++)
		{
			OCTNode node;
			OCTNode::deserialize(node, &octReader);

			if (node.childCount == 0xCDCDu && node.childIndex == 0xCDCDu && node.objectIndex == 0xCDCDu)
			{
				// Alignment node. Skip and break
				break;
			}

			// Store max index of requested object to obtain count of objects at all
			if (node.objectIndex > totalObjects)
				totalObjects = node.objectIndex;

			m_nodes.emplace_back(node);
		}

		// Because previously totalObjects was index, not a count
		++totalObjects;

		// Read objects
		// Seek to begin
		octReader.seek(m_header.objectsOffset);
		m_objects.resize(totalObjects);

		for (int i = 0; i < totalObjects; i++)
		{
			OCTObject::deserialize(m_objects[i], &octReader);
		}

		// And last block...
		m_unknownBlocks.resize(totalObjects);

		for (int i = 0; i < totalObjects; i++)
		{
			OCTUnknownBlock::deserialize(m_unknownBlocks[i], &octReader);
		}

		return true;
	}
}