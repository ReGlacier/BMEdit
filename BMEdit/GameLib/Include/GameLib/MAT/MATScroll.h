#pragma once

#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATScroll
	{
	public:
		MATScroll(std::string name, bool bEnabled, std::vector<float>&& speedVector);

		static MATScroll makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const { return m_name; }
		[[nodiscard]] bool isEnabled() const { return m_bEnabled; }
		[[nodiscard]] const std::vector<float>& getSpeedVec() const { return m_vfSpeed; }

	private:
		std::string m_name {};
		bool m_bEnabled { false };
		std::vector<float> m_vfSpeed {};
	};
}