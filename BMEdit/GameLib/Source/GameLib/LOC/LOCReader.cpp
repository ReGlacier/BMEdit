#include <GameLib/LOC/LOCReader.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>


namespace gamelib::loc
{
	LOCReader::LOCReader() = default;

	bool LOCReader::parse(const uint8_t *locFileBuffer, int64_t locFileSize)
	{
		// First node is always ROOT and starts from 1 byte 0x1
		ZBio::ZBinaryReader::BinaryReader reader { (const char*)locFileBuffer, (int64_t)locFileSize };

		// Check first byte
		auto rootChildNr = reader.read<uint8_t>();
		if (rootChildNr < 1) return false; // no root nodes found

		// Create pseudo ROOT
		m_pRoot = std::make_shared<LOCTreeNode>();
		m_pRoot->type = LOCTreeNodeType::CHILDREN;
		m_pRoot->name = "ROOT";

		std::vector<uint32_t> offsets {};
		offsets.resize(rootChildNr);
		offsets[0] = 0x0; // zero offset

		for (int i = 0; i < rootChildNr - 1; i++)
		{
			offsets[i + 1] = reader.read<uint32_t>();
		}

		// Create children
		m_pRoot->children.resize(rootChildNr);

		for (int i = 0; i < rootChildNr; i++)
		{
			ZBioSeekGuard guard { &reader };
			ZBioHelpers::seekBy(&reader, offsets[i]);

			m_pRoot->children[i] = std::make_shared<LOCTreeNode>();
			m_pRoot->children[i]->parent = m_pRoot; // store parent

			LOCTreeNode::deserialize(m_pRoot->children[i], &reader);
		}

		return true;
	}

	const LOCTreeNode::Ptr& LOCReader::getRoot() const
	{
		return m_pRoot;
	}
}