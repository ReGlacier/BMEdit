#pragma once

#include <GameLib/PRP/PRPRegionID.h>
#include <GameLib/PRP/PRPException.h>


namespace gamelib::prp
{
	class PRPBadInstruction final : public PRPException
	{
	public:
		using PRPException::PRPException;
	};
}