#include <GameLib/PRP/PRPException.h>
#include <sstream>


namespace gamelib::prp
{
	PRPException::PRPException(const std::string &message, PRPRegionID region, int opCodeIndex)
		: std::exception(message.data())
	{
		std::stringstream ss;

		ss << "PRP Structure error (at region ";

		switch (region) {
		case PRPRegionID::HEADER: ss << "'header'";
			break;
		case PRPRegionID::TOKEN_TABLE: ss << "'token table'";
			break;
		case PRPRegionID::ZDEFINITIONS: ss << "'zdefs'";
			break;
		case PRPRegionID::INSTRUCTIONS: ss << "'instructions'";
			break;
		case PRPRegionID::UNKNOWN:
		default: ss << "'unknown'";
			break;
		}

		ss << "; opc index is ";

		if (opCodeIndex >= 0) {
			ss << opCodeIndex << " ";
		}
		else {
			ss << " not recognized ";
		}

		ss << "): " << message;

		m_errorMessage = ss.str();
	}

	const char *PRPException::what() const
	{
		return m_errorMessage.data();
	}
}