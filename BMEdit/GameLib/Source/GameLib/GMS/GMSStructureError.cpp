#include <GameLib/GMS/GMSStructureError.h>


namespace gamelib::gms
{
	GMSStructureError::GMSStructureError(const char *message) : std::exception(message)
	{
	}
}