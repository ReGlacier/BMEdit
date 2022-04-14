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

	private:
		bool loadLevelProperties();
		bool loadLevelScene();

	private:
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
		std::vector<std::unique_ptr<scene::SceneObject>> m_sceneObjects;
		LevelProperties m_levelProperties;
		SceneProperties m_sceneProperties;
		bool m_isLevelLoaded { false };
	};
}