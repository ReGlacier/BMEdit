#pragma once

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
		MATOption(std::string name, bool bEnabled, bool bDefault);

		static MATOption makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] bool getDefault() const;


	private:
		std::string m_name{};
		bool m_bEnabled {};
		bool m_bDefault {};
	};
}