#pragma once

#include <GameLib/MAT/MATValU.h>
#include <string>
#include <variant>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATOption
	{
	public:
		MATOption(std::string name, bool bEnabled, MATValU&& valU);

		static MATOption makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] const MATValU& getValU() const;


	private:
		std::string m_name{};
		bool m_bEnabled {};
		MATValU m_valU {};
	};
}