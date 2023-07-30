#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRM/PRM.h>
#include <GameLib/PRP/PRP.h>
#include <GameLib/GMS/GMS.h>
#include <GameLib/TEX/TEX.h>

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

	struct LevelTextures
	{
		tex::TEXHeader header;
		std::vector<tex::TEXEntry> entries;
		tex::OffsetsPool table1Offsets { 0u };
		tex::OffsetsPool table2Offsets { 0u };
		uint32_t countOfEmptyOffsets { 0u };
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
		[[nodiscard]] const LevelGeometry* getLevelGeometry() const;
		[[nodiscard]] LevelGeometry* getLevelGeometry();
		[[nodiscard]] const LevelTextures* getSceneTextures() const;
		[[nodiscard]] LevelTextures* getSceneTextures();

		[[nodiscard]] const std::vector<scene::SceneObject::Ptr> &getSceneObjects() const;

		void dumpAsset(io::AssetKind assetKind, std::vector<uint8_t> &outBuffer) const;

	private:
		bool loadLevelProperties();
		bool loadLevelScene();
		bool loadLevelPrimitives();
		bool loadLevelTextures();

	private:
		// Core
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
		bool m_isLevelLoaded { false };

		// Raw data
		LevelProperties m_levelProperties;
		SceneProperties m_sceneProperties;
		LevelGeometry m_levelGeometry;
		LevelTextures m_levelTextures;

		// Managed objects
		std::vector<scene::SceneObject::Ptr> m_sceneObjects {};
	};
}