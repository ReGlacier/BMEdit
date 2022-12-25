#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRM/PRM.h>
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

	struct LevelGeometry
	{
		prm::PRMHeader header;
		std::vector<prm::PRMChunkDescriptor> chunkDescriptors;
		std::vector<prm::PRMChunk> chunks;
	};

	struct SceneProperties
	{
		gms::GMSHeader header;
	};

	using LoadLevelOptions = std::uint16_t;

	enum LoadLevelOption : LoadLevelOptions
	{
		LLO_NONE = 0,
		LLO_SKIP_SCENE_TREE = 1 << 0,
		LLO_SKIP_PROPERTIES = 1 << 1,
		LLO_SKIP_GEOMETRY = 1 << 2,
	};

	class Level
	{
	public:
		explicit Level(std::unique_ptr<io::IOLevelAssetsProvider> &&levelAssetsProvider);

		[[nodiscard]] bool loadSceneData(LoadLevelOptions options = LoadLevelOption::LLO_NONE);

		[[nodiscard]] const std::string &getLevelName() const;
		[[nodiscard]] const LevelProperties *getLevelProperties() const;
		[[nodiscard]] LevelProperties *getLevelProperties();
		[[nodiscard]] const SceneProperties *getSceneProperties() const;
		[[nodiscard]] const LevelGeometry* getLevelGeometry() const;
		[[nodiscard]] LevelGeometry* getLevelGeometry();

		[[nodiscard]] const std::vector<scene::SceneObject::Ptr> &getSceneObjects() const;

		void dumpAsset(io::AssetKind assetKind, std::vector<uint8_t> &outBuffer) const;

	private:
		bool loadLevelProperties();
		bool loadLevelScene();
		bool loadLevelPrimitives();

	private:
		// Core
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
		bool m_isLevelLoaded { false };

		// Raw data
		LevelProperties m_levelProperties;
		SceneProperties m_sceneProperties;
		LevelGeometry m_levelGeometry;

		// Managed objects
		std::vector<scene::SceneObject::Ptr> m_sceneObjects {};
	};
}