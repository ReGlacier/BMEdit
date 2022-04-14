#include <Editor/ZIPLevelAssetProvider.h>
#include <string_view>
#include <filesystem>


namespace editor
{
	static constexpr std::string_view kAssetExtensions[gamelib::io::AssetKind::LAST_ASSET_KIND] = {
		"GMS", "PRP", "TEX", "PRM", "MAT", "OCT", "RMI", "RMC", "LOC", "ANM", "SND", "BUF", "ZGF"
	};

	ZIPLevelAssetProvider::ZIPLevelAssetProvider(std::string containerPath) : m_path(std::move(containerPath))
	{
		m_zipHandle = unzOpen(m_path.data());
	}

	ZIPLevelAssetProvider::~ZIPLevelAssetProvider()
	{
		if (m_zipHandle)
		{
			unzClose(m_zipHandle);
			m_zipHandle = nullptr;
		}
	}

	std::unique_ptr<uint8_t []> ZIPLevelAssetProvider::getAsset(gamelib::io::AssetKind kind, int64_t &bufferSize) const
	{
		if (!m_zipHandle)
		{
			return nullptr;
		}

		if (unzGoToFirstFile(m_zipHandle) != UNZ_OK)
		{
			return nullptr;
		}

		do
		{
			static constexpr int kMaxFileNameLength = 256;
			char fileName[kMaxFileNameLength] = { 0 };
			unz_file_info fileInfo;

			auto ret = unzGetCurrentFileInfo(m_zipHandle, &fileInfo, fileName, kMaxFileNameLength, nullptr, 0, nullptr, 0);
			if (ret != UNZ_OK)
			{
				return nullptr;
			}

			/**
			 * Here we have a filename and we would like to check that our 'filename' ends with our extension.
			 * In most cases our extensions are in upper case and this function wants to work with upper case
			 */
			std::string_view currentFileName { fileName };
			if (currentFileName.ends_with(kAssetExtensions[kind]))
			{
				// File found, need to take filename, save it as level name and read contents
				std::filesystem::path currentFilePath { currentFileName };
				m_levelName = currentFilePath.filename().string();

				// Switch to current file
				ret = unzOpenCurrentFile(m_zipHandle);
				if (ret != UNZ_OK)
				{
					return nullptr;
				}

				auto buffer = std::make_unique<uint8_t[]>(fileInfo.uncompressed_size);
				int readBytes = unzReadCurrentFile(m_zipHandle, buffer.get(), fileInfo.uncompressed_size);

				if (readBytes < 0)
				{
					//TODO: Throw exception
					return nullptr;
				}

				bufferSize = readBytes;
				if (fileInfo.uncompressed_size != bufferSize)
				{
					//TODO: Warning
				}

				return buffer;
			}

			ret = unzGoToNextFile(m_zipHandle);
			if (ret != UNZ_OK)
			{
				return nullptr;
			}
		}
		while (true);
	}

	std::string ZIPLevelAssetProvider::getLevelName() const
	{
		if (m_levelName.empty() && m_zipHandle)
		{
			// Try to locate & cache actual value
			int64_t bs = 0;
			(void)getAsset(gamelib::io::AssetKind::ZGF, bs);
		}

		return m_levelName;
	}
}