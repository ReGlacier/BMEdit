#include <GameLib/LOC/LOCTreeNode.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>
#include <cassert>


namespace gamelib::loc
{
	void LOCTreeNode::deserialize(const gamelib::loc::LOCTreeNode::Ptr &node, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		// Read self name
		node->name = binaryReader->readCString();

		const auto type = binaryReader->read<int8_t, ZBio::Endianness::LE>();
		if (type != LOCTreeNodeType::CHILDREN && type != LOCTreeNodeType::LOCALIZED_STRING && type != LOCTreeNodeType::SUBTITLES && type != LOCTreeNodeType::EMPTY_BLOCK)
		{
			throw std::runtime_error("Invalid LOC format: expected to have 0x0 or 0x10, got smth else");
		}

		// Store type
		node->type = static_cast<LOCTreeNodeType>(type);

		if (node->type == LOCTreeNodeType::CHILDREN)
		{
			// Read amount
			auto amount = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
			if (amount)
			{
				node->children.reserve(amount);

				std::vector<uint32_t> offsets {};
				offsets.resize(amount);
				offsets[0] = 0x0; // zero offset since next nodes

				for (int i = 1; i < amount; i++)
				{
					offsets[i] = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
				}

				// Now we've ready to iterate over offsets and read child nodes
				for (const auto& offset : offsets)
				{
					ZBioSeekGuard guard { binaryReader };
					ZBioHelpers::seekBy(binaryReader, offset);

					auto childNode = std::make_shared<LOCTreeNode>();
					childNode->parent = node;

					// Read node
					LOCTreeNode::deserialize(childNode, binaryReader);

					// Save node
					node->children.emplace_back(childNode);
				}
			}
		}
		else if (node->type == LOCTreeNodeType::LOCALIZED_STRING)
		{
			// Read string value
			node->value = binaryReader->readCString();
			ZBioHelpers::seekBy(binaryReader, 4); // Need seek by 4 because value strings are "aligned".
			// Formula: len + 1 + 4 (+1 - zero terminator, 4 - "alignment")
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES)
		{
			// Subtitles text
			node->value = binaryReader->readCString();

			// Read subtitle data. I really don't know what that value means, but as 2xu32 each same to each
			binaryReader->read<uint8_t, ZBio::Endianness::LE>(&node->subtitle.unkData[0], 8);

			// Check that next byte is always valid for us
			{
				ZBioSeekGuard guard { binaryReader };
				auto lb = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
				assert((lb >= 'a' && lb <='z') || (lb >= 'A' && lb <= 'Z') || (lb >= '0' && lb <= '9'));
			}
		}
		else if (node->type == LOCTreeNodeType::EMPTY_BLOCK)
		{
			// Do nothing here, it's just empty
		}
		else
		{
			throw std::runtime_error { "Unsupported block" };
		}
	}

	void LOCTreeNode::serialize(const LOCTreeNode::Ptr &node, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter) // NOLINT(*-no-recursion)
	{
		// Write name (not aligned)
		binaryWriter->writeCString(node->name);

		// Write node type
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(node->type));

		if (node->type == LOCTreeNodeType::CHILDREN)
		{
			// Write count
			const auto childrenCount = node->children.size() > 0xFF ? 0xFF : static_cast<uint8_t>(node->children.size());
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(childrenCount);

			// Write offsets
			std::vector<uint32_t> offsets{};

			for (int i = 1; i < childrenCount; i++)
			{
				offsets.push_back(binaryWriter->tell());
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(0); // Fake offset (will be fixed)
			}

			// Write node by node & restore offsets
			const uint32_t baseOffset = binaryWriter->tell();

			for (int i = 0; i < childrenCount; i++)
			{
				if (i > 0)
				{
					// Restore offset
					uint32_t newOffset = binaryWriter->tell();

					ZBioSeekGuard<ZBio::ZBinaryWriter::BinaryWriter> guard { binaryWriter };
					binaryWriter->seek(offsets[i - 1]);

					// Update offset
					binaryWriter->write<uint32_t, ZBio::Endianness::LE>(newOffset - baseOffset); // Offset calculated from node #0
				}

				// Write node itself
				LOCTreeNode::serialize(node->children[i], binaryWriter);
			}
		}
		else if (node->type == LOCTreeNodeType::LOCALIZED_STRING)
		{
			// Write string
			binaryWriter->writeCString(node->value);

			// Write 4 byte alignment
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(0);
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES)
		{
			// Write unaligned string
			binaryWriter->writeCString(node->value);

			// Write subtitles data
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(&node->subtitle.unkData[0], 8);
		}
		else if (node->type == LOCTreeNodeType::EMPTY_BLOCK)
		{
			// Do nothing here
		}
	}
}