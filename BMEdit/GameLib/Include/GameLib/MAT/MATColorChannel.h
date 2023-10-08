#pragma once

#include <string>
#include <variant>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	using MATColorRGBA = std::variant<glm::vec4, glm::ivec4, glm::vec3, glm::ivec3>;

	/**
	 * @brief Present option for color channel (able to pass rgba as float & int values)
	 */
	class MATColorChannel
	{
	public:
		MATColorChannel(std::string name, bool bEnabled, MATColorRGBA&& color);

		static MATColorChannel makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] const MATColorRGBA& getColor() const;

	private:
		std::string m_name {};
		bool m_bEnabled { false };
		MATColorRGBA m_color {};
	};
}