#include <Widgets/EditorToolFactory.h>

// Widgets
#include <SelectSceneObjectTool.h>
#include <SelectScriptTool.h>
#include <QDebug>


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

		if (hintId == "SelectGameScript")
		{
			pResult = SelectScriptTool::Create(parent);
		}

		if (!pResult)
		{
			qWarning() << "For hint '" << QString::fromStdString(hintId) << "' no editor created. Check name please";
		}

		return pResult;
	}
}