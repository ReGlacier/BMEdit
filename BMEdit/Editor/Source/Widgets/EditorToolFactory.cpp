#include <Widgets/EditorToolFactory.h>

// Widgets
#include <SelectSceneObjectTool.h>


namespace widgets
{
	// Base factory
	TypePropertyWidget *EditorToolFactory::createToolById(QWidget* parent, const std::string &hintId)
	{
		TypePropertyWidget* pResult = nullptr;
		if (hintId == "SelectGeomTool")
		{
			pResult = SelectSceneObjectTool::Create(parent);
		}

		return pResult;
	}
}