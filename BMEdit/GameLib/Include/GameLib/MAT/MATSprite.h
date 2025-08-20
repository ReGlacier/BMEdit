#pragma once

#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATSprite
	{
	public:
		MATSprite(std::string name, bool bUnk0, bool bUnk1);

		static MATSprite makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool getUnk0() const;
		[[nodiscard]] bool getUnk1() const;

	private:
		std::string m_name {};
		bool m_bUnk0 { false };
		bool m_bUnk1 { false };
	};
}