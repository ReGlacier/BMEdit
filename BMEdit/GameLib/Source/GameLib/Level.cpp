#include <GameLib/GMS/GMSReader.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/Level.h>
#include <GameLib/PRP/PRPReader.h>

#include <GameLib/Scene/SceneObjectPropertiesLoader.h>
#include <GameLib/Scene/SceneObjectPropertiesDumper.h>

#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/Type.h>

#include <sstream>
#include <string>
#include <vector>


namespace gamelib
{
	glm::vec3 LevelRooms::RoomGroup::worldToRoom(const glm::vec3& vWorld) const
	{
		return {
			(vWorld.x - header.vWorldOrigin.x) * header.fWorldScale + 32768.0f,
			(vWorld.y - header.vWorldOrigin.y) * header.fWorldScale + 32768.0f,
			(vWorld.z - header.vWorldOrigin.z) * header.fWorldScale + 32768.0f
		};
	}

	glm::vec3 LevelRooms::RoomGroup::roomToWorld(const glm::vec3& vRoom) const
	{
		return {
		    (vRoom.x - 32768.0f) / header.fWorldScale + header.vWorldOrigin.x,
		    (vRoom.y - 32768.0f) / header.fWorldScale + header.vWorldOrigin.y,
		    (vRoom.z - 32768.0f) / header.fWorldScale + header.vWorldOrigin.z
		};
	}

	Level::Level(std::unique_ptr<io::IOLevelAssetsProvider> &&levelAssetsProvider)
		: m_assetProvider(std::move(levelAssetsProvider))
	{
	}

	bool Level::loadSceneData()
	{
		if (!m_assetProvider || !m_assetProvider->isValid())
		{
			return false;
		}

		if (!loadLevelProperties())
		{
			return false;
		}

		if (!loadLevelScene())
		{
			return false;
		}

		if (!loadLevelPrimitives())
		{
			return false;
		}

		if (!loadLevelTextures())
		{
			return false;
		}

		if (!loadLevelMaterials())
		{
			return false;
		}

		if (!loadLevelRooms())
		{
			return false;
		}

		// TODO: Load things (it's time to combine GMS, PRP & BUF files)
		m_isLevelLoaded = true;
		return true;
	}

	const std::string &Level::getLevelName() const
	{
		if (m_assetProvider)
		{
			return m_assetProvider->getLevelName();
		}

		static std::string kUnknownLevel = "Unknown";
		return kUnknownLevel;
	}

	const LevelProperties *Level::getLevelProperties() const
	{
		return m_isLevelLoaded ? &m_levelProperties : nullptr;
	}

	LevelProperties *Level::getLevelProperties()
	{
		return m_isLevelLoaded ? &m_levelProperties : nullptr;
	}

	const SceneProperties *Level::getSceneProperties() const
	{
		if (m_isLevelLoaded)
		{
			return &m_sceneProperties;
		}

		return nullptr;
	}

	const LevelTextures* Level::getSceneTextures() const
	{
		return &m_levelTextures;
	}

	LevelTextures* Level::getSceneTextures()
	{
		return &m_levelTextures;
	}


	const LevelGeometry* Level::getLevelGeometry() const
	{
		return &m_levelGeometry;
	}

	LevelGeometry* Level::getLevelGeometry()
	{
		return &m_levelGeometry;
	}

	const LevelMaterials* Level::getLevelMaterials() const
	{
		return &m_levelMaterials;
	}

	LevelMaterials* Level::getLevelMaterials()
	{
		return &m_levelMaterials;
	}

	const std::vector<scene::SceneObject::Ptr> &Level::getSceneObjects() const
	{
		return m_sceneObjects;
	}

	[[nodiscard]] scene::SceneObject::Ptr Level::getSceneObjectByGEOMREF(const std::string& path) const
	{
		std::stringstream pathStream { path };
		std::string segment;
		std::vector<std::string> dividedPath {};

		while(std::getline(pathStream, segment, '\\'))
		{
			dividedPath.push_back(segment);
		}

		scene::SceneObject::Ptr object = m_sceneObjects[0];

		for (const auto& pathBlock : dividedPath)
		{
			if (pathBlock == "ROOT")
				continue;

			bool bFound = false;

			for (const auto& childrenRef : object->getChildren())
			{
				if (auto child = childrenRef.lock(); child && child->getName() == pathBlock)
				{
					bFound = true;
					object = child;
					break;
				}
			}

			if (!bFound)
			{
				return nullptr;
			}
		}

		return object;
	}

	void Level::dumpAsset(io::AssetKind assetKind, std::vector<uint8_t> &outBuffer) const
	{
		if (assetKind == io::AssetKind::PROPERTIES)
		{
			scene::SceneObjectPropertiesDumper dumper;
			dumper.dump(this, &outBuffer);
		}
	}

	void Level::forEachObjectOfType(const std::string& objectTypeName, const std::function<bool(const scene::SceneObject::Ptr&)>& pred) const
	{
		for (const auto& object: m_sceneObjects)
		{
			if (object->getType()->getName() == objectTypeName)
			{
				if (pred(object))
					continue;

				return;
			}
		}
	}

	bool isComplexTypeInheritedOf(const std::string& baseTypeName, const TypeComplex* pType)
	{
		return pType != nullptr && (pType->getName() == baseTypeName || isComplexTypeInheritedOf(baseTypeName, reinterpret_cast<const TypeComplex*>(pType->getParent())));
	}

	void Level::forEachObjectOfTypeWithInheritance(const std::string& objectBaseType, const std::function<bool(const scene::SceneObject::Ptr&)>& pred) const
	{
		for (const auto& object: m_sceneObjects)
		{
			if (object->getType()->getKind() == TypeKind::COMPLEX && isComplexTypeInheritedOf(objectBaseType, reinterpret_cast<const TypeComplex*>(object->getType())))
			{
				if (pred(object))
					continue;

				return;
			}
		}
	}

	bool Level::loadLevelProperties()
	{
		int64_t prpFileSize = 0;
		auto prpFileBuffer = m_assetProvider->getAsset(io::AssetKind::PROPERTIES, prpFileSize);
		if (!prpFileBuffer || !prpFileSize)
		{
			return false;
		}

		prp::PRPReader reader;
		if (!reader.parse(prpFileBuffer.get(), prpFileSize))
		{
			return false;
		}

		m_levelProperties.header = reader.getHeader();
		m_levelProperties.objectsCount = reader.getObjectsCount();
		m_levelProperties.rawProperties = reader.getByteCode().getInstructions();
		m_levelProperties.ZDefines = reader.getDefinitions();
		return true;
	}

	bool Level::loadLevelScene()
	{
		int64_t gmsFileSize = 0;
		int64_t bufFileSize = 0;

		// Load raw data
		auto gmsFileBuffer = m_assetProvider->getAsset(io::AssetKind::SCENE, gmsFileSize);
		if (!gmsFileBuffer || !gmsFileSize)
		{
			return false;
		}

		auto bufFileBuffer = m_assetProvider->getAsset(io::AssetKind::BUFFER, bufFileSize);
		if (!bufFileBuffer || !bufFileSize)
		{
			return false;
		}

		gms::GMSReader reader;
		if (!reader.parse(&m_sceneProperties.header, gmsFileBuffer.get(), gmsFileSize, bufFileBuffer.get(), bufFileSize))
		{
			return false;
		}

		// Load abstract scene objects
		const auto &entities = m_sceneProperties.header.getEntries().getGeomEntities();
		if (!entities.empty())
		{
			m_sceneObjects.resize(entities.size());

			// Create objects
			for (std::size_t sceneObjectIndex = 0; sceneObjectIndex < entities.size(); ++sceneObjectIndex)
			{
				scene::SceneObject::Instructions propertyInstructions {};
				auto& currentGeom = entities[sceneObjectIndex];

				auto geomTypeId = currentGeom.getTypeId();
				auto geomType = TypeRegistry::getInstance().findTypeByHash(geomTypeId);

				m_sceneObjects[sceneObjectIndex] = std::make_shared<scene::SceneObject>(
				    currentGeom.getName(),
				    geomTypeId,
				    geomType,
				    currentGeom,
				    propertyInstructions
			    );
			}

			// Visit properties
			using scene::SceneObject;
			using scene::SceneObject;

			scene::SceneObjectPropertiesLoader::load(Span(m_sceneObjects), Span(m_levelProperties.rawProperties));

			// Scene hierarchy setup
			for (const auto& sceneObject : m_sceneObjects)
			{
				auto parentIndex= sceneObject->getGeomInfo().getParentGeomIndex();
				if (parentIndex == gms::GMSGeomEntity::kInvalidParent)
				{
					continue; // No parent, probably ROOT
				}

				sceneObject->setParent(m_sceneObjects[parentIndex]);
			}

#if 0       //TODO: Remove this code later
			std::int32_t lowestPrimId = 0xFFFF;

			for (const auto& sceneObj: m_sceneObjects)
			{
				if (TypeRegistry::canCast<"ZGEOM">(sceneObj->getType()))
				{
					auto primId = sceneObj->getProperties()["PrimId"][0].getOperand().get<std::int32_t>();

					if (primId == 0)
						continue;

					if (primId < lowestPrimId)
						lowestPrimId = primId;
				}
			}
			printf("Found minimal primId: %d\n", lowestPrimId);
#endif
		}

		return true;
	}

	bool Level::loadLevelPrimitives()
	{
		// Read PRM file
		int64_t prmFileSize = 0;
		auto prmFileBuffer = m_assetProvider->getAsset(gamelib::io::AssetKind::GEOMETRY, prmFileSize);

		if (!prmFileSize || !prmFileBuffer)
		{
			return false;
		}

		PRMReader reader;
		if (!reader.parse(prmFileBuffer.get(), prmFileSize))
		{
			return false;
		}

		m_levelGeometry.primitives = std::move(reader.takePrimitives());

		return true;
	}

	bool Level::loadLevelTextures()
	{
		// Read TEX file
		int64_t texFileSize = 0;
		auto texFileBuffer = m_assetProvider->getAsset(gamelib::io::AssetKind::TEXTURES, texFileSize);

		if (!texFileSize || !texFileBuffer)
		{
			return false;
		}

		tex::TEXReader reader;
		const bool parseResult = reader.parse(texFileBuffer.get(), texFileSize);
		if (!parseResult)
		{
			return false;
		}

		m_levelTextures.header = reader.m_header;
		m_levelTextures.entries = std::move(reader.m_entries);
		m_levelTextures.table1Offsets = reader.m_texturesPool;
		m_levelTextures.table2Offsets = reader.m_cubeMapsPool;
		m_levelTextures.countOfEmptyOffsets = reader.m_countOfEmptyOffsets;

		return true;
	}

	bool Level::loadLevelMaterials()
	{
		// Read MAT file
		int64_t matFileSize = 0;
		auto matFileBuffer = m_assetProvider->getAsset(gamelib::io::AssetKind::MATERIALS, matFileSize);

		if (!matFileSize || !matFileBuffer)
		{
			return false;
		}

		mat::MATReader reader;
		const bool parseResult = reader.parse(matFileBuffer.get(), matFileSize);
		if (!parseResult)
		{
			return false;
		}

		m_levelMaterials.header = reader.getHeader();
		m_levelMaterials.materialClasses = std::move(reader.takeClasses());
		m_levelMaterials.materialInstances = std::move(reader.takeInstances());

		return true;
	}

	bool Level::loadLevelRooms()
	{
		// Read outside rooms
		{
			int64_t rmcFileSize = 0;
			auto rmcFileBuffer = m_assetProvider->getAsset(gamelib::io::AssetKind::ROOM_TREE_OUTSIDE, rmcFileSize);

			if (!rmcFileBuffer || !rmcFileSize)
			{
				return false;
			}

			oct::OCTReader rmcReader {};
			const bool bParseResult = rmcReader.parse(rmcFileBuffer.get(), rmcFileSize);

			if (!bParseResult)
			{
				return false;
			}

			m_levelRooms.outside.header = rmcReader.getHeader();
			m_levelRooms.outside.nodes = std::move(rmcReader.takeNodes());
			m_levelRooms.outside.objects = std::move(rmcReader.takeObjects());
			m_levelRooms.outside.ubs = std::move(rmcReader.takeUBS());
		}

		// Read inside rooms
		{
			int64_t rmcFileSize = 0;
			auto rmcFileBuffer = m_assetProvider->getAsset(gamelib::io::AssetKind::ROOM_TREE_INSIDE, rmcFileSize);

			if (!rmcFileBuffer || !rmcFileSize)
			{
				return false;
			}

			oct::OCTReader rmcReader {};
			const bool bParseResult = rmcReader.parse(rmcFileBuffer.get(), rmcFileSize);

			if (!bParseResult)
			{
				return false;
			}

			m_levelRooms.inside.header = rmcReader.getHeader();
			m_levelRooms.inside.nodes = std::move(rmcReader.takeNodes());
			m_levelRooms.inside.objects = std::move(rmcReader.takeObjects());
			m_levelRooms.inside.ubs = std::move(rmcReader.takeUBS());
		}

		// Read collisions
		{
		}

		return true;
	}
}