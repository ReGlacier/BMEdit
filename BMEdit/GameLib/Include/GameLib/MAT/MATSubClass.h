#pragma once

#include <GameLib/MAT/MATLayer.h>
#include <string>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATSubClass
	{
	public:
		MATSubClass(std::string name, std::string oTyp, std::string sType, std::vector<MATLayer>&& layers)
		    	: m_name(std::move(name)), m_oTyp(std::move(oTyp)), m_sTyp(std::move(sType)), m_layers(std::move(layers))
		{
		}

		static MATSubClass makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const { return m_name; }
		[[nodiscard]] const std::string& getOTyp() const { return m_oTyp; }
		[[nodiscard]] const std::string& getSTyp() const { return m_sTyp; }
		[[nodiscard]] const std::vector<MATLayer>& getLayers() const { return m_layers; }

	private:
		std::string m_name {};
		std::string m_oTyp {};
		std::string m_sTyp {};
		std::vector<MATLayer> m_layers {};
	};
}