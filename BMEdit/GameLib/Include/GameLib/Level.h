#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRP.h>
#include <GameLib/GMS/GMS.h>

#include <memory>
#include <vector>
#include <cstdint>


namespace gamelib
{
	struct LevelProperties
	{
		prp::PRPHeader header;
		prp::PRPZDefines ZDefines;
		std::vector<prp::PRPInstruction> rawProperties;
		uint32_t objectsCount;
	};

	struct SceneProperties
	{
		gms::GMSHeader header;
	};

	class Level
	{
	public:
		explicit Level(std::unique_ptr<io::IOLevelAssetsProvider> &&levelAssetsProvider);

		[[nodiscard]] bool loadSceneData();

		[[nodiscard]] const std::string &getLevelName() const;
		[[nodiscard]] const LevelProperties *getLevelProperties() const;
		[[nodiscard]] LevelProperties *getLevelProperties();
		[[nodiscard]] const SceneProperties *getSceneProperties() const;

		[[nodiscard]] const std::vector<scene::SceneObject::Ptr> &getSceneObjects() const;

	private:
		bool loadLevelProperties();
		bool loadLevelScene();

	private:
		// Core
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
		bool m_isLevelLoaded { false };

		// Raw data
		LevelProperties m_levelProperties;
		SceneProperties m_sceneProperties;

		// Managed objects
		std::vector<scene::SceneObject::Ptr> m_sceneObjects {};
	};
}