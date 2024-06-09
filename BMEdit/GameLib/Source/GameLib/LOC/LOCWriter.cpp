#include <GameLib/LOC/LOCWriter.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryWriter.hpp>


namespace gamelib::loc
{
	void LOCWriter::write(const LOCTreeNode::Ptr &root, std::vector<uint8_t> &outBuffer)
	{
		if (!root || root->children.empty()) return;

		auto writerSink = std::make_unique<ZBio::ZBinaryWriter::BufferSink>();
		auto binaryWriter = ZBio::ZBinaryWriter::BinaryWriter(std::move(writerSink));

		// Sort children by name. It's required because game wants to interact with sorted tree
		auto children = root->children;
		std::sort(children.begin(), children.end(), [](const LOCTreeNode::Ptr& a, const LOCTreeNode::Ptr& b) { return a->name < b->name; });

		// Write base
		binaryWriter.write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(children.size()));

		std::vector<uint32_t> offsets {};

		// Fill offsets if root child more than 1
		for (int i = 1; i < children.size(); i++)
		{
			// Here we need to know an offset of subject after first created node. Instead of that we will save current offset and then jump back
			offsets.push_back(binaryWriter.tell());
			binaryWriter.write<uint32_t, ZBio::Endianness::LE>(0); // Temp value, will be replaced later
		}

		// And now we've ready to write node by node
		const uint32_t baseOffset = binaryWriter.tell(); // Save current offset (need to calculate relative offset)

		for (int i = 0; i < children.size(); i++)
		{
			if (i > 0)
			{
				// Save current offset
				uint32_t newOffset = binaryWriter.tell();

				// Seek back to origin
				ZBioSeekGuard<ZBio::ZBinaryWriter::BinaryWriter> guard { &binaryWriter };
				binaryWriter.seek(offsets[i - 1]);

				// Update offset
				binaryWriter.write<uint32_t, ZBio::Endianness::LE>(newOffset - baseOffset); // Offset calculated from node #0
			}

			// Write node itself
			LOCTreeNode::serialize(children[i], &binaryWriter);
		}

		// Done
		auto raw = binaryWriter.release().value();
		std::copy(raw.begin(), raw.end(), std::back_inserter(outBuffer));
	}
}