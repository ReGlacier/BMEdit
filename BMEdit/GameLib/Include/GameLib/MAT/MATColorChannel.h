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
	/**
	 * @brief Present option for color channel (able to pass rgba as float & int values)
	 */
	class MATColorChannel
	{
	public:
		MATColorChannel(std::string name, bool bEnabled, MATValU&& color);

		static MATColorChannel makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] const MATValU& getColor() const;

	private:
		std::string m_name {};
		bool m_bEnabled { false };
		MATValU m_color {};
	};
}