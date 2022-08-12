#pragma once

#include <stdexcept>
#include <cstdint>

#include <string>


namespace gamelib::scene
{
	class SceneObjectVisitorException : public std::exception
	{
	protected:
		std::string m_errorDesc;
		std::string m_cachedWhat;
		uint32_t m_objectId;

	public:
		SceneObjectVisitorException(uint32_t objectId, std::string  errorDescription);

		[[nodiscard]] char const* what() const override;
	};
}