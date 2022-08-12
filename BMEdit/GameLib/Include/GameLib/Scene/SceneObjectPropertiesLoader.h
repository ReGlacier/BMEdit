#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>

#include <GameLib/Span.h>
#include <GameLib/Value.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/PRP/PRPBadInstruction.h>


namespace gamelib::scene
{
	class SceneObjectPropertiesLoader
	{
	public:
		static void load(Span<SceneObject::Ptr> objects, Span<prp::PRPInstruction> instructions);
	};
}