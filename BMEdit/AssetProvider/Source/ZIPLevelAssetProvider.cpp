#include <AssetProvider/ZIPLevelAssetProvider.h>
#include <string_view>
#include <filesystem>
#include <cassert>

extern "C"
{
#include <zip.h>
}

#ifndef BMEDIT_DEBUG
	#define BMEDIT_ZIP_REPORT_ERROR(ze)
#else
	#define BMEDIT_ZIP_REPORT_ERROR(ze)                           \
	{                                                         \
		printf("ZIP error: %s\n", zip_error_strerror(&(ze))); \
		assert(false);                                        \
	}
#endif


namespace ap
{
	struct ZIPLevelAssetProvider::Context
	{
		zip_t* m_archive { nullptr };
		zip_source_t* m_source{ nullptr };
		zip_error_t m_lastError{};
		std::string m_path {};
		std::string m_levelName;
		std::unordered_map<gamelib::io::AssetKind, std::string> m_assetNamesCache;
		bool m_isOk { true };

		~Context()
		{
			if (m_archive)
			{
				zip_close(m_archive);
				m_archive = nullptr;
			}

			if (m_source)
			{
				zip_source_close(m_source);
				m_source = nullptr;
			}

			m_isOk = false;
		}

		[[nodiscard]] bool isValid() const
		{
			return m_isOk && m_archive && m_source;
		}
	};

	static constexpr int IOI_FILE_NAME_LIMIT = 512;
	static constexpr std::string_view kAssetExtensions[gamelib::io::AssetKind::LAST_ASSET_KIND] = {
	    "GMS", "PRP", "TEX", "PRM", "MAT", "OCT", "RMI", "RMC", "LOC", "ANM", "SND", "BUF", "ZGF"
	};

	bool filePathEndsWith(std::string_view fileName, std::string_view extension)
	{
		if (!fileName.ends_with(extension))
		{
			// Try to convert out extension to lowercase and check filename again
			char tmpBuffer[8] { 0 };
			if (extension.length() < sizeof(tmpBuffer))
			{
				int i = 0;
				for (const auto& ch: extension)
				{
					tmpBuffer[i] = static_cast<char>(std::tolower(ch));
					++i;
				}

				return fileName.ends_with(&tmpBuffer[0]);
			}

			assert(false && "Too long file extension!");
			return false;
		}

		return true;
	}

	ZIPLevelAssetProvider::ZIPLevelAssetProvider(std::string containerPath)
	{
		m_ctx = std::make_unique<Context>();
		m_ctx->m_path = std::move(containerPath);
		{
			zip_error_t zipError;
			zip_error_init(&m_ctx->m_lastError);
			m_ctx->m_source = zip_source_file_create(m_ctx->m_path.data(), 0, 0, &zipError);

			if (m_ctx->m_source)
			{
				m_ctx->m_archive = zip_open_from_source(m_ctx->m_source, 0, &m_ctx->m_lastError);
				if (!m_ctx->m_archive)
				{
					BMEDIT_ZIP_REPORT_ERROR(m_ctx->m_lastError);
				}
			}
			else
			{
				BMEDIT_ZIP_REPORT_ERROR(m_ctx->m_lastError);
			}
		}
		m_ctx->m_isOk = m_ctx->m_source && m_ctx->m_archive;
	}

	ZIPLevelAssetProvider::~ZIPLevelAssetProvider() = default;

	std::unique_ptr<uint8_t []> ZIPLevelAssetProvider::getAsset(gamelib::io::AssetKind kind, int64_t &bufferSize) const
	{
		zip_int64_t numEntries = zip_get_num_entries(m_ctx->m_archive, 0);
		if (numEntries <= 0)
		{
			return nullptr;
		}

		for (zip_int64_t entryIndex = 0; entryIndex < numEntries; ++entryIndex)
		{
			const char* entryNameRaw = zip_get_name(m_ctx->m_archive, entryIndex, ZIP_FL_ENC_GUESS);
			if (!entryNameRaw)
			{
				assert(false && "Failed to extract entry name");
				continue;
			}

			std::string_view entryName { entryNameRaw };

			if (filePathEndsWith(entryName, kAssetExtensions[kind]))
			{
				if (m_ctx->m_levelName.empty()) // Cache level name
				{
					std::filesystem::path entryPath { entryNameRaw };
					m_ctx->m_levelName = entryPath.stem().string();
				}

				// File found! Need to read it
				zip_stat_t zipFileInfo;

				int res = zip_stat_index(m_ctx->m_archive, entryIndex, ZIP_STAT_NAME | ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_FLAGS, &zipFileInfo);
				if (res < 0)
				{
					assert(false && "Failed to extract zip index!");
					return nullptr; // Failed to retrieve information about zip
				}

				bufferSize = static_cast<int64_t>(zipFileInfo.size);
				auto buffer = std::make_unique<uint8_t[]>(bufferSize);
				if (!buffer)
				{
					assert(false && "Failed to allocate memory");
					return nullptr; // Unable to allocate buffer
				}

				zip_file_t* zipFile = zip_fopen_index(m_ctx->m_archive, entryIndex, 0);
				if (!zipFile)
				{
					assert(false && "Failed to open file in archive");
					return nullptr;
				}

				zip_int64_t readyBytes = zip_fread(zipFile, buffer.get(), bufferSize);
				assert(readyBytes == zipFileInfo.size && "Invalid rdy bytes count");

				if (!readyBytes)
				{
					assert(false && "Failed to read file contents");
					return nullptr;
				}

				zip_fclose(zipFile);
				return buffer;
			}
		}

		return nullptr;
	}

	const std::string &ZIPLevelAssetProvider::getLevelName() const
	{
		if (!m_ctx)
		{
			static const std::string kInvalid;
			return kInvalid;
		}

		if (m_ctx->m_levelName.empty() && m_ctx->isValid())
		{
			// Try to locate & cache actual value
			int64_t bs = 0;
			(void)getAsset(gamelib::io::AssetKind::ZGF, bs);
		}

		return m_ctx->m_levelName;
	}

	bool ZIPLevelAssetProvider::hasAssetOfKind(gamelib::io::AssetKind kind) const
	{
		return !getAssetFileName(kind).empty();
	}

	bool ZIPLevelAssetProvider::saveAsset(gamelib::io::AssetKind kind, gamelib::Span<uint8_t> assetBody)
	{
		if (!isValid())
		{
			return false;
		}

		zip_int64_t totalEntriesNum = zip_get_num_entries(m_ctx->m_archive, 0);
		for (zip_int64_t entryIndex = 0; entryIndex < totalEntriesNum; ++entryIndex)
		{
			const char* entryNameRaw = zip_get_name(m_ctx->m_archive, entryIndex, ZIP_FL_ENC_GUESS);
			if (!entryNameRaw)
			{
				assert(false && "Failed to extract name of entry in archive");
				continue;
			}

			std::string_view entryName { entryNameRaw };
			if (filePathEndsWith(entryName, kAssetExtensions[kind]))
			{
				auto fileSource = zip_source_buffer(m_ctx->m_archive, assetBody.data(), static_cast<zip_int64_t>(assetBody.size()), 0);
				if (!fileSource)
				{
					assert(false);
					return false;
				}

				const int replaceResult = zip_file_replace(m_ctx->m_archive, entryIndex, fileSource, ZIP_FL_ENC_UTF_8);
				if (replaceResult != 0)
				{
					zip_source_free(fileSource); // Should be released here
					assert(false && "Failed to replace file");
					return false;
				}

				return true;
			}
		}

		return false;
	}

	bool ZIPLevelAssetProvider::isValid() const
	{
		return m_ctx && m_ctx->isValid();
	}

	bool ZIPLevelAssetProvider::isEditable() const
	{
		return isValid();
	}

	std::string ZIPLevelAssetProvider::getAssetFileName(gamelib::io::AssetKind kind) const
	{
		if (!isValid()) return {};

		if (auto it = m_ctx->m_assetNamesCache.find(kind); it != m_ctx->m_assetNamesCache.end())
		{
			return it->second;
		}

		zip_int64_t numEntries = zip_get_num_entries(m_ctx->m_archive, 0);
		if (numEntries <= 0)
		{
			return {};
		}

		for (zip_int64_t entryIndex = 0; entryIndex < numEntries; ++entryIndex)
		{
			const char* entryNameRaw = zip_get_name(m_ctx->m_archive, entryIndex, ZIP_FL_ENC_GUESS);
			if (!entryNameRaw)
			{
				assert(false && "Failed to extract name of entry in archive");
				continue;
			}

			std::string_view entryName { entryNameRaw };
			if (filePathEndsWith(entryName, kAssetExtensions[kind]))
			{
				auto& fNameRes = m_ctx->m_assetNamesCache[kind];
				fNameRes.reserve(entryName.size());
				std::copy(entryName.begin(), entryName.end(), std::back_inserter(fNameRes));
				return fNameRes;
			}
		}

		return {};
	}
}

// Undefs
#undef BMEDIT_ZIP_REPORT_ERROR