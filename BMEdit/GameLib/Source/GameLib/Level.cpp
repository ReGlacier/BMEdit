#include <GameLib/Level.h>
#include <GameLib/Type.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/PRP/PRPReader.h>
#include <GameLib/GMS/GMSReader.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/Scene/SceneObjectPropertiesVisitor.h>


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
		if (m_isLevelLoaded)
		{
			return &m_levelProperties;
		}

		return nullptr;
	}

	const SceneProperties *Level::getSceneProperties() const
	{
		if (m_isLevelLoaded)
		{
			return &m_sceneProperties;
		}

		return nullptr;
	}

	const std::vector<scene::SceneObject::Ptr> &Level::getSceneObjects() const
	{
		return m_sceneObjects;
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

				if (auto parentGeomIndex = currentGeom.getParentGeomIndex(); parentGeomIndex != gms::GMSGeomEntity::kInvalidParent) {
					m_sceneObjects[sceneObjectIndex]->setParent(m_sceneObjects[parentGeomIndex]);
					m_sceneObjects[parentGeomIndex]->getChildren().push_back(m_sceneObjects[sceneObjectIndex]);
				}
			}

			// Visit properties
			scene::SceneObjectPropertiesVisitor::visit(
			    m_sceneObjects,
			    Span<prp::PRPInstruction> { m_levelProperties.rawProperties });
		}

		return true;
	}
}