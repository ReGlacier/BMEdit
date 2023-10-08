#include <GameLib/MAT/MATLayer.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATLayer::MATLayer(std::string name, std::string type, std::string shaderPath, std::string identity, std::string val_i)
		: m_name(std::move(name)), m_type(std::move(type)), m_shaderPath(std::move(shaderPath)), m_identity(std::move(identity)), m_valI(std::move(val_i))
	{
	}

	const std::string& MATLayer::getName() const
	{
		return m_name;
	}

	const std::string& MATLayer::getType() const
	{
		return m_type;
	}

	const std::string& MATLayer::getShaderProgramName() const
	{
		return m_shaderPath;
	}

	const std::string& MATLayer::getIdentity() const
	{
		return m_identity;
	}

	MATLayer MATLayer::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		assert(propertiesCount > 0 && "Bad props count");
		assert(binaryReader != nullptr && "Bad reader");

		std::string name {}, type {}, shaderPath {}, identity {}, valI {};

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			// Save pos & seek to value
			ZBioSeekGuard guard { binaryReader };
			binaryReader->seek(entry.reference);

			// Read value by type
			if (auto kind = entry.kind; kind == MATPropertyKind::PK_NAME)
			{
				name = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_TYPE)
			{
				type = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_PATH)
			{
				shaderPath = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_IDEN)
			{
				identity = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_VAL_I)
			{
				valI = binaryReader->readCString();
			}
		}

		return MATLayer(std::move(name), std::move(type), std::move(shaderPath), std::move(identity), std::move(valI));
	}
}