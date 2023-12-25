#pragma once

#include <GameLib/MAT/MATValU.h>
#include <variant>
#include <string>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	using IntOrFloat = std::variant<float, uint32_t>;

	class MATFloat
	{
	public:
		MATFloat(std::string name, bool bEnabled, MATValU&& valU);

		static MATFloat makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const { return m_name; }
		[[nodiscard]] bool isEnabled() const { return m_bEnabled; }
		[[nodiscard]] const MATValU& getValU() const { return m_valU; }

	private:
		std::string m_name {};
		bool m_bEnabled { false };
		MATValU m_valU;
	};
}