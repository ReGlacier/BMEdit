#pragma once

#include <vector>

#include <GameLib/Span.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRPInstruction.h>


namespace gamelib::scene
{
	struct SceneObjectPropertiesVisitor
	{
		static void visit(const std::vector<SceneObject::Ptr> &sceneObjects, const Span<prp::PRPInstruction> &properties);

	private:
		static void visit(Span<SceneObject::Ptr> &sceneObjects, Span<prp::PRPInstruction> &properties);
	};
}