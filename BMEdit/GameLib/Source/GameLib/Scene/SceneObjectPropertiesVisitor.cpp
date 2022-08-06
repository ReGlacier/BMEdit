#include <GameLib/Scene/SceneObjectPropertiesVisitor.h>
#include <GameLib/PRP/PRPBadInstruction.h>


namespace gamelib::scene
{
	using namespace gamelib::prp;

	void SceneObjectPropertiesVisitor::visit(const std::vector<SceneObject::Ptr> &sceneObjects, const Span<PRPInstruction> &properties)
	{
		Span<PRPInstruction> currentInstructions { properties };
		Span<SceneObject::Ptr> sceneObjectsSpan { sceneObjects };

		SceneObjectPropertiesVisitor::visit(sceneObjectsSpan, currentInstructions);
	}

	void SceneObjectPropertiesVisitor::visit(Span<SceneObject::Ptr> &sceneObjects, Span<prp::PRPInstruction> &properties)
	{
		auto &currentObject = sceneObjects[0];

		if (!properties[0].isBeginObject())
		{
			throw PRPBadInstruction("Expected BeginObject/BeginNamedObject", PRPRegionID::INSTRUCTIONS);
		}

		++properties;

		// Visit general properties
		auto mappingResult = currentObject->getType()->map(properties);
		printf("PIZDEC\n");

		// Visit controllers

		// Visit children
	}
}