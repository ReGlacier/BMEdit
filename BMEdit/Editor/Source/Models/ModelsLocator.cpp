#include <Models/ModelsLocator.h>


namespace models
{
	std::unique_ptr<SceneObjectsTreeModel> ModelsLocator::s_SceneTreeModel { nullptr };
	std::unique_ptr<GameScriptsTreeModel> ModelsLocator::s_GameScriptsTreeModel { nullptr };
}