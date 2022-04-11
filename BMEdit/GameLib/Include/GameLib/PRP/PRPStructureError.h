#pragma once

#include <stdexcept>

#include <GameLib/PRP/PRPException.h>


namespace gamelib::prp {
	class PRPStructureError final : public PRPException {
	public:
		using PRPException::PRPException;
	};
}