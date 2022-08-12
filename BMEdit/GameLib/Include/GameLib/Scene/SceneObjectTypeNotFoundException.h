#pragma once

#include <GameLib/SCene/SceneObjectVisitorException.h>


namespace gamelib::scene
{
	class SceneObjectTypeNotFoundException : public SceneObjectVisitorException
	{
	public:
		SceneObjectTypeNotFoundException(uint32_t objectIdx, uint32_t typeHash);
		SceneObjectTypeNotFoundException(uint32_t objectIdx, const std::string& typeName);
	};
}