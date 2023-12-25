#pragma once

#include <GameLib/MAT/MATEntries.h>
#include <GameLib/Span.h>
#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	/**
	 * @brief Present layer from Glacier material system
	 */
	class MATLayer
	{
	public:
		MATLayer(std::string name, std::string type, std::string shaderPath, std::string identity, std::string val_i);

		static MATLayer makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] const std::string& getType() const;
		[[nodiscard]] const std::string& getShaderProgramName() const;
		[[nodiscard]] const std::string& getIdentity() const;
	private:
		std::string m_name;
		std::string m_type;
		std::string m_shaderPath;
		std::string m_identity;
		std::string m_valI;
	};
}