#include <Include/IO/ZIPLevelAssetProvider.h>
#include <string_view>
#include <filesystem>

extern "C"
{
#include <zip.h>
}


namespace editor
{
	static constexpr int IOI_FILE_NAME_LIMIT = 512;
	static constexpr std::string_view kAssetExtensions[gamelib::io::AssetKind::LAST_ASSET_KIND] = {
		"GMS", "PRP", "TEX", "PRM", "MAT", "OCT", "RMI", "RMC", "LOC", "ANM", "SND", "BUF", "ZGF"
	};

	class ZipHandlerBackup
	{
		unzFile* m_handle;
		std::string m_path;
		bool m_shouldRestore{ true };
	public:
		ZipHandlerBackup(const std::string &path, unzFile* handle)
			: m_handle(handle), m_path(path), m_shouldRestore(handle && *handle)
		{
			unzClose(*handle);
			*handle = nullptr;
		}

		~ZipHandlerBackup()
		{
			if (m_shouldRestore)
			{
				*m_handle = unzOpen(m_path.data());
			}
		}

		ZipHandlerBackup(const ZipHandlerBackup&) = delete;
		ZipHandlerBackup(ZipHandlerBackup&&) = delete;
		ZipHandlerBackup& operator=(const ZipHandlerBackup&) = delete;
		ZipHandlerBackup& operator=(ZipHandlerBackup&&) = delete;
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
			static constexpr int kMaxFileNameLength = IOI_FILE_NAME_LIMIT;
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
				m_levelName = currentFilePath.stem().string();

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

	const std::string &ZIPLevelAssetProvider::getLevelName() const
	{
		if (m_levelName.empty() && m_zipHandle)
		{
			// Try to locate & cache actual value
			int64_t bs = 0;
			(void)getAsset(gamelib::io::AssetKind::ZGF, bs);
		}

		return m_levelName;
	}

	bool ZIPLevelAssetProvider::hasAssetOfKind(gamelib::io::AssetKind kind) const
	{
		return !getAssetFileName(kind).empty();
	}

	bool ZIPLevelAssetProvider::saveAsset(gamelib::io::AssetKind kind, gamelib::Span<uint8_t> assetBody)
	{
		if (!isEditable())
		{
			//TODO: Throw exception?
			return false;
		}

		auto fileName = getAssetFileName(kind);
		if (fileName.empty())
		{
			//TODO: Throw exception?
			return false;
		}

		{
			ZipHandlerBackup handlerBackup { m_path, &m_zipHandle }; // From this point until scope finished zip handle will be invalid
			zipFile archive = zipOpen(m_path.data(), APPEND_STATUS_CREATEAFTER); //APPEND_STATUS_ADDINZIP);
			if (!archive)
			{
				//TODO: Throw exception?
				return false;
			}


			zip_fileinfo zipFileInfo;
			int res = zipOpenNewFileInZip(archive, fileName.data(), &zipFileInfo, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
			if (res != ZIP_OK)
			{
				//TODO: Throw exception
				return false;
			}

			res = zipWriteInFileInZip(archive, assetBody.data(), assetBody.size());
			if (res != ZIP_OK)
			{
				//TODO: Throw exception
				return false;
			}

			zipCloseFileInZip(archive);
			zipClose(archive, nullptr);
		}
		return true;
	}

	bool ZIPLevelAssetProvider::isValid() const
	{
		return m_zipHandle != nullptr;
	}

	bool ZIPLevelAssetProvider::isEditable() const
	{
		return isValid();
	}

	std::string ZIPLevelAssetProvider::getAssetFileName(gamelib::io::AssetKind kind) const
	{
		if (!m_zipHandle)
		{
			return {};
		}

		if (auto it = m_assetNamesCache.find(kind); it != m_assetNamesCache.end())
		{
			return it->second;
		}

		if (unzGoToFirstFile(m_zipHandle) != UNZ_OK)
		{
			return {};
		}

		do
		{
			static constexpr int kMaxFileNameLength = IOI_FILE_NAME_LIMIT;
			char fileName[kMaxFileNameLength] = { 0 };
			unz_file_info fileInfo;

			auto ret = unzGetCurrentFileInfo(m_zipHandle, &fileInfo, fileName, kMaxFileNameLength, nullptr, 0, nullptr, 0);
			if (ret != UNZ_OK)
			{
				return {};
			}

			/**
			 * Here we have a filename and we would like to check that our 'filename' ends with our extension.
			 * In most cases our extensions are in upper case and this function wants to work with upper case
			 */
			std::string_view currentFileName { fileName };
			if (currentFileName.ends_with(kAssetExtensions[kind]))
			{
				auto& fNameRes = m_assetNamesCache[kind];
				fNameRes.reserve(currentFileName.size());
				std::copy(currentFileName.begin(), currentFileName.end(), std::back_inserter(fNameRes));

				return fNameRes;
			}

			ret = unzGoToNextFile(m_zipHandle);
			if (ret != UNZ_OK)
			{
				return {};
			}
		}
		while (true);

		return {};
	}
}