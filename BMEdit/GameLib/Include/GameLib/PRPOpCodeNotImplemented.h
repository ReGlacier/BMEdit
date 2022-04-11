#pragma once

#include <GameLib/PRPException.h>


namespace gamelib::prp
{
	class PRPOpCodeNotImplemented final : public PRPException
	{
	public:
		using PRPException::PRPException;
	};
}