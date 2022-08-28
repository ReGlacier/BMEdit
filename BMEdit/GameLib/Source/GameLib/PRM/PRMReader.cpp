#include <GameLib/PRM/PRMReader.h>
#include <GameLib/PRM/PRMPrimitiveDescriptor.h>
#include <GameLib/PRM/PRMPrimitive.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	bool PRMReader::read(Span<uint8_t> buffer)
	{
		if (!buffer)
		{
			return false;
		}

		ZBio::ZBinaryReader::BinaryReader binaryReader(reinterpret_cast<const char*>(buffer.data()), buffer.size());

		// Read header
		PRMHeader::deserialize(m_header, &binaryReader);
		if (m_header.zeroed != 0x0)
		{
			assert(false && "Zeroed field must be zeroed!");
			return false;
		}

		if (m_header.countOfPrimitives >= 40960) // There are 40960 geoms max. Need to check it later
		{
			assert(false && "Possibly invalid PRM file. Game supports max 40959 unique primitives per level");
			return false;
		}

		//See g_aPrimChunks in IDA DB for details
		for (int primIndex = 0; primIndex < m_header.countOfPrimitives; ++primIndex)
		{
			// Read primitive descriptor
			binaryReader.seek(m_header.chunkOffset + (primIndex * PRMPrimitiveDescriptor::kDescriptorSize));
			PRMPrimitiveDescriptor descriptor {};
			PRMPrimitiveDescriptor::deserialize(descriptor, &binaryReader);

			// Read primitive declaration
			auto &newPrim = m_primitives.emplace_back();
			binaryReader.seek(descriptor.declarationOffset);
			PRMPrimitive::deserialize(newPrim, &binaryReader);
			//TODO: Prepare primitive
		}

		return true;
	}

	const PRMHeader &PRMReader::getHeader() const
	{
		return m_header;
	}

	const std::vector<PRMPrimitive> &PRMReader::getPrimitives() const
	{
		return m_primitives;
	}
}