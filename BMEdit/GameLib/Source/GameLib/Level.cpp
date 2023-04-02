#include <GameLib/GMS/GMSReader.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/Level.h>
#include <GameLib/PRP/PRPReader.h>

#include <GameLib/Scene/SceneObjectPropertiesLoader.h>
#include <GameLib/Scene/SceneObjectPropertiesDumper.h>

#include <GameLib/Type.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/PRM/PRMReader.h>


namespace gamelib
{
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

	const LevelGeometry *Level::getLevelGeometry() const
	{
		return &m_levelGeometry;
	}

	LevelGeometry *Level::getLevelGeometry()
	{
		return &m_levelGeometry;
	}

	const std::vector<scene::SceneObject::Ptr> &Level::getSceneObjects() const
	{
		return m_sceneObjects;
	}

	void Level::dumpAsset(io::AssetKind assetKind, std::vector<uint8_t> &outBuffer) const
	{
		if (assetKind == io::AssetKind::PROPERTIES)
		{
			scene::SceneObjectPropertiesDumper dumper;
			dumper.dump(this, &outBuffer);
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

		prm::PRMReader reader { m_levelGeometry.header, m_levelGeometry.chunkDescriptors, m_levelGeometry.chunks };
		if (!reader.read(Span(prmFileBuffer.get(), prmFileSize)))
		{
			return false;
		}

		return true;
	}
}