#include <GameLib/LOC/LOCTreeNode.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>
#include <fmt/format.h>
#include <cassert>


namespace gamelib::loc
{
	bool LOCTreeNode::canHaveValue() const
	{
		return
		    type == LOCTreeNodeType::LOCALIZED_STRING ||
		    type == LOCTreeNodeType::SUBTITLES ||
		    type == LOCTreeNodeType::SUBTITLES_HINT ||
		    type == LOCTreeNodeType::SUBTITLES_FIN ||
		    type == LOCTreeNodeType::SUBTITLES_HINT_WITH_COMMENT;
	}

	bool LOCTreeNode::canHaveChildren() const
	{
		return type == LOCTreeNodeType::CHILDREN;
	}

	void LOCTreeNode::deserialize(const gamelib::loc::LOCTreeNode::Ptr &node, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		// Read self name
		node->name = binaryReader->readCString();

		const auto type = binaryReader->read<int8_t, ZBio::Endianness::LE>();
		if (type != LOCTreeNodeType::CHILDREN && type != LOCTreeNodeType::LOCALIZED_STRING &&
		    type != LOCTreeNodeType::SUBTITLES && type != LOCTreeNodeType::EMPTY_BLOCK &&
		    type != LOCTreeNodeType::SUBTITLES_FIN && type != LOCTreeNodeType::SUBTITLES_HINT &&
		    type != LOCTreeNodeType::SUBTITLES_HINT_WITH_COMMENT)
		{
			auto errorMessage = fmt::format("Invalid LOC format: unexpected entity code 0x{:02X} at offset {} (0x{:X})", type, binaryReader->tell(), binaryReader->tell());
			throw std::runtime_error(errorMessage);
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

			// Read subtitle data. In most cases there are 2xu32, but when first u32 zeroed next u32 not presented
			// I love IOI because they don't give me an opportunity to relax...
			uint32_t first = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			uint32_t second = 0;
			if (first != 0)
			{
				// Ok, read second
				second = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			}

			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]) = first;
			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[4]) = second;
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES_HINT)
		{
			// Subtitles text
			node->value = binaryReader->readCString();

			// Read extra hint string
			node->subtitle.extraHint = binaryReader->readCString();

			// Read subtitle data. In most cases there are 2xu32, but when first u32 zeroed next u32 not presented
			// I love IOI because they don't give me an opportunity to relax...
			uint32_t first = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			uint32_t second = 0;
			if (first != 0)
			{
				// Ok, read second
				second = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			}

			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]) = first;
			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[4]) = second;
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES_FIN)
		{
			// Always only 1 u32
			uint32_t first = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]) = first;
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES_HINT_WITH_COMMENT)
		{
			// Read value
			node->value = binaryReader->readCString();

			// Read hint (?)
			node->subtitle.extraHint = binaryReader->readCString();

			// DronCode: I'm not sure that next few bytes always zeroed.
			// As we remember in LOCTreeNodeType::SUBTITLES & LOCTreeNodeType::SUBTITLES_HINT we have extra 8 bytes (2xu32 but sometimes only 1xu32 when it's zeroed).
			// This could be our case. Idk, let's use code from SUBTITLES_FIN (idk why)
			uint32_t first = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			*reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]) = first;
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

	void LOCTreeNode::serialize(const LOCTreeNode::Ptr &node, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter, std::vector<std::pair<size_t, uint32_t>>& replacement) // NOLINT(*-no-recursion)
	{
		// Write name (not aligned)
		binaryWriter->writeCString(node->name);

		// Write node type
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(node->type));

		if (node->type == LOCTreeNodeType::CHILDREN)
		{
			// Sort children
			auto children = node->children; // Need copy
			//std::sort(children.begin(), children.end(), [](const LOCTreeNode::Ptr& a, const LOCTreeNode::Ptr& b) { return a->name < b->name; });

			// Write count
			const auto childrenCount = children.size() > 0xFF ? 0xFF : static_cast<uint8_t>(children.size());
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(childrenCount);

			// Write offsets
			std::vector<uint32_t> offsets { 0u };

			for (int i = 1; i < childrenCount; i++)
			{
				offsets.push_back(binaryWriter->tell());
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(0xDDDDDDDDu);
			}

			// Write node by node & restore offsets
			const uint32_t baseOffset = binaryWriter->tell();

			for (int i = 0; i < childrenCount; i++)
			{
				if (i > 0)
				{
					// Save replacement instruction
					uint32_t newOffset = binaryWriter->tell();
					replacement.emplace_back(offsets[i], newOffset - baseOffset);
				}

				// Write node itself
				LOCTreeNode::serialize(children[i], binaryWriter, replacement);
			}
		}
		else if (node->type == LOCTreeNodeType::LOCALIZED_STRING)
		{
			// Write string
			binaryWriter->writeCString(node->value);

			// Write 4 byte alignment
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(0);
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES || node->type == LOCTreeNodeType::SUBTITLES_HINT)
		{
			// Write unaligned string
			binaryWriter->writeCString(node->value);

			if (node->type == LOCTreeNodeType::SUBTITLES_HINT)
			{
				// For hinted string need to write extra hint
				binaryWriter->writeCString(node->subtitle.extraHint);
			}

			uint32_t first = *reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]);
			uint32_t second = *reinterpret_cast<uint32_t*>(&node->subtitle.unkData[4]);

			// Write subtitles data
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(first);
			if (first)
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(second);
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES_HINT_WITH_COMMENT)
		{
			// Write value
			binaryWriter->writeCString(node->value);

			// Write hint
			binaryWriter->writeCString(node->subtitle.extraHint);

			// Write u32 (see deserializer code for details)
			uint32_t first = *reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]);
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(first);
		}
		else if (node->type == LOCTreeNodeType::SUBTITLES_FIN)
		{
			// Store tutorial data here
			uint32_t first = *reinterpret_cast<uint32_t*>(&node->subtitle.unkData[0]);

			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(first);
		}
		else if (node->type == LOCTreeNodeType::EMPTY_BLOCK)
		{
			// Do nothing here
		}
	}
}