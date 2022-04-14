#include <GameLib/Level.h>
#include <GameLib/PRP/PRPReader.h>
#include <GameLib/GMS/GMSReader.h>


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
		auto gmsFileBuffer = m_assetProvider->getAsset(io::AssetKind::SCENE, gmsFileSize);
		if (!gmsFileBuffer || !gmsFileSize)
		{
			return false;
		}

		gms::GMSReader reader;
		if (!reader.parse(gmsFileBuffer.get(), gmsFileSize))
		{
			return false;
		}

		return true;
	}
}