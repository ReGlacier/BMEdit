#include <GameLib/GMS/GMSStructureError.h>


namespace gamelib::gms
{
	GMSStructureError::GMSStructureError(std::string message)
		: std::exception("GMSStructureError")
		, m_message(std::move(message))
	{
	}

	const char *GMSStructureError::what() const noexcept
	{
		return m_message.data();
	}
}