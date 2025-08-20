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

		std::vector<std::pair<size_t, uint32_t>> replacement {};

		// Sort children by name. It's required because game wants to interact with sorted tree
		auto children = root->children;
		//std::sort(children.begin(), children.end(), [](const LOCTreeNode::Ptr& a, const LOCTreeNode::Ptr& b) { return a->name < b->name; });

		// Write base
		binaryWriter.write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(children.size()));

		std::vector<uint32_t> offsets { 0u }; // zero stored by default

		// Fill offsets if root child more than 1
		for (int i = 1; i < children.size(); i++)
		{
			// Here we need to know an offset of subject after first created node. Instead of that we will save current offset and then jump back
			offsets.push_back(binaryWriter.tell());
			binaryWriter.write<uint32_t, ZBio::Endianness::LE>(0xDDDDDDDDu); // Temp value, will be replaced later
		}

		// And now we've ready to write node by node
		const uint32_t baseOffset = binaryWriter.tell(); // Save current offset (need to calculate relative offset)

		for (int i = 0; i < children.size(); i++)
		{
			if (i > 0)
			{
				// Save replacement
				uint32_t newOffset = binaryWriter.tell();
				replacement.emplace_back(offsets[i], newOffset - baseOffset);
			}

			// Write node itself
			LOCTreeNode::serialize(children[i], &binaryWriter, replacement);
		}

		// Take value
		auto raw = binaryWriter.release().value();

		// Apply replacements
		for (const auto& [offset, value] : replacement)
		{
			*reinterpret_cast<uint32_t*>(raw.data() + offset) = value;
		}

		// Done
		std::copy(raw.begin(), raw.end(), std::back_inserter(outBuffer));
	}
}