#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRP.h>
#include <GameLib/GMS/GMS.h>
#include <GameLib/TEX/TEX.h>
#include <GameLib/PRM/PRM.h>
#include <GameLib/MAT/MAT.h>
#include <GameLib/OCT/OCT.h>

#include <functional>
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

	struct LevelGeometry
	{
		prm::PrmFile primitives;
	};

	struct LevelMaterials
	{
		mat::MATHeader header;
		std::vector<mat::MATClass> materialClasses;
		std::vector<mat::MATInstance> materialInstances;
	};

	struct LevelRooms
	{
		struct RoomGroup
		{
			oct::OCTHeader header {};
			std::vector<oct::OCTNode> nodes{};
			std::vector<oct::OCTObject> objects{};
			std::vector<oct::OCTUnknownBlock> ubs{};

			[[nodiscard]] glm::vec3 worldToRoom(const glm::vec3& vWorld) const;
			[[nodiscard]] glm::vec3 roomToWorld(const glm::vec3& vTree) const;
		};

		RoomGroup outside {};
		RoomGroup inside {};
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
		[[nodiscard]] const LevelTextures* getSceneTextures() const;
		[[nodiscard]] LevelTextures* getSceneTextures();
		[[nodiscard]] const LevelGeometry* getLevelGeometry() const;
		[[nodiscard]] LevelGeometry* getLevelGeometry();
		[[nodiscard]] const LevelMaterials* getLevelMaterials() const;
		[[nodiscard]] LevelMaterials* getLevelMaterials();

		[[nodiscard]] const std::vector<scene::SceneObject::Ptr>& getSceneObjects() const;

		void dumpAsset(io::AssetKind assetKind, std::vector<uint8_t> &outBuffer) const;

		void forEachObjectOfType(const std::string& objectTypeName, const std::function<bool(const scene::SceneObject::Ptr&)>& pred) const;
		void forEachObjectOfTypeWithInheritance(const std::string& objectBaseType, const std::function<bool(const scene::SceneObject::Ptr&)>& pred) const;

	private:
		bool loadLevelProperties();
		bool loadLevelScene();
		bool loadLevelPrimitives();
		bool loadLevelTextures();
		bool loadLevelMaterials();
		bool loadLevelRooms();

	private:
		// Core
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
		bool m_isLevelLoaded { false };

		// Raw data
		LevelProperties m_levelProperties;
		SceneProperties m_sceneProperties;
		LevelTextures m_levelTextures;
		LevelGeometry m_levelGeometry;
		LevelMaterials m_levelMaterials;
		LevelRooms m_levelRooms;

		// Managed objects
		std::vector<scene::SceneObject::Ptr> m_sceneObjects {};
	};
}