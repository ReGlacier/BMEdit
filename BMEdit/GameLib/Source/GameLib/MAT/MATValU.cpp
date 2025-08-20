#include <GameLib/MAT/MATValU.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>



namespace gamelib::mat
{
	MATValU::MATValU() = default;

	MATValU::MATValU(std::vector<Value>&& values) : m_values(std::move(values))
	{
	}

	MATValU &MATValU::operator=(std::vector<Value> &&values) noexcept
	{
		m_values = std::move(values);
		return *this;
	}

	MATValU MATValU::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, const MATPropertyEntry& selfDecl)
	{
		assert(selfDecl.kind == MATPropertyKind::PK_VAL_U);

		std::vector<Value> values {};
		values.resize(selfDecl.containerCapacity);

		if (selfDecl.containerCapacity == 1)
		{
			// Parse single value
			if (selfDecl.valueType == MATValueType::PT_UINT32)
			{
				// Single uint32
				values[0] = static_cast<uint32_t>(selfDecl.reference);
			}
			else if (selfDecl.valueType == MATValueType::PT_FLOAT)
			{
				// Single float
				values[0] = static_cast<float>(selfDecl.reference);
			}
			else
			{
				assert(false && "Unsupported & impossible case!");
			}
		}
		else
		{
			// Parse group
			ZBioSeekGuard guard { binaryReader };
			binaryReader->seek(selfDecl.reference);

			for (int i = 0; i < selfDecl.containerCapacity; i++)
			{
				if (selfDecl.valueType == MATValueType::PT_UINT32)
				{
					values[i] = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
				}
				else if (selfDecl.valueType == MATValueType::PT_FLOAT)
				{
					values[i] = binaryReader->read<float, ZBio::Endianness::LE>();
				}
				else
				{
					assert(false && "Unsupported & impossible case!");
				}
			}
		}

		return MATValU(std::move(values));
	}
}