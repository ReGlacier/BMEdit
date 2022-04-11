#pragma once

#include <GameLib/PRPException.h>


namespace gamelib::prp {
	class PRPBadStringReference final : public PRPException
	{
	public:
		using PRPException::PRPException;
	};
}