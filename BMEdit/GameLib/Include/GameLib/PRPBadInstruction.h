#pragma once

#include <GameLib/PRPRegionID.h>
#include <GameLib/PRPException.h>


namespace gamelib::prp
{
	class PRPBadInstruction final : public PRPException
	{
	public:
		using PRPException::PRPException;
	};
}