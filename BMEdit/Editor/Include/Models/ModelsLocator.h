#pragma once

#include <memory>

#include <Models/SceneObjectsTreeModel.h>


namespace models
{
	/**
	 * @brief This class holds all sharable models to share them between dialogs/widgets/etc.
	 */
	struct ModelsLocator final
	{
		ModelsLocator() = default;
		ModelsLocator(const ModelsLocator&) = delete;
		ModelsLocator(ModelsLocator&&) = delete;


		// Instances
		static std::unique_ptr<SceneObjectsTreeModel> s_SceneTreeModel;
	};
}