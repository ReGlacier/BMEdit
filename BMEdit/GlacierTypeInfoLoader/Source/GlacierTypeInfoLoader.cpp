#include <GTIL/GlacierTypeInfoLoader.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeNotFoundException.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <sstream>
#include <fstream>


namespace gtil
{
	const ErrorDescription ErrorDescription::kNoError { ErrorCode::EC_NO_ERROR, "" };

	ErrorDescription GlacierTypeInfoLoader::loadTypes(const std::filesystem::path &typesIndex)
	{
		gamelib::TypeRegistry::getInstance().reset();

		std::stringstream contentsBuffer;

		{
			std::ifstream typeRegistryFile(typesIndex, std::ios::in);
			if (!typeRegistryFile.is_open())
			{
				return { ErrorCode::EC_FAILED_TO_OPEN_INDEX, "Failed to load file" };
			}

			contentsBuffer << typeRegistryFile.rdbuf();
		}

		auto registryFile = nlohmann::json::parse(contentsBuffer.str(), nullptr, false, true);
		if (registryFile.is_discarded()) {
			return { ErrorCode::EC_INVALID_INDEX_JSON, "Invalid json format!" };
		}

		if (!registryFile.contains("inc") || !registryFile.contains("db"))
		{
			return { ErrorCode::EC_INVALID_INDEX_SCHEMA, "Invalid types database schema" };
		}

		auto &registry = gamelib::TypeRegistry::getInstance();

		std::unordered_map<std::string, std::string> typesToHashes;
		for (const auto &[hash, typeNameObj]: registryFile["db"].items())
		{
			typesToHashes[typeNameObj.get<std::string>()] = hash;
		}

		const auto incPath = registryFile["inc"].get<std::string>();
		const std::filesystem::path incDir { typesIndex.parent_path() / std::filesystem::path { incPath } };

		// Here we need to scan for all .json files in 'inc' folder and parse 'em all
		std::vector<nlohmann::json> typeInfos;

		for (const auto &dirEntry : std::filesystem::recursive_directory_iterator { std::filesystem::path { incDir } })
		{
			if (!dirEntry.is_regular_file())
				continue;

			std::ifstream typeDescriptionFile { dirEntry.path(), std::ios::in };
			if (!typeDescriptionFile.is_open())
			{
				return { ErrorCode::EC_UNABLE_TO_OPEN_TYPE_ENTRY, fmt::format("Failed to open type description file: {}", dirEntry.path().string()) };
			}

			std::stringstream typeDescriptionBuffer;
			typeDescriptionBuffer << typeDescriptionFile.rdbuf();

			auto &jsonContents = typeInfos.emplace_back();
			jsonContents = nlohmann::json::parse(typeDescriptionBuffer.str(), nullptr, false, true);
			if (jsonContents.is_discarded())
			{
				return { ErrorCode::EC_INVALID_TYPE_ENTRY_JSON, fmt::format("Failed to parse entry declaration: {}", dirEntry.path().string()) };
			}
		}

		try
		{
			registry.registerTypes(std::move(typeInfos), std::move(typesToHashes));

			return ErrorDescription::kNoError;
		}
		catch (const gamelib::TypeNotFoundException &typeNotFoundException)
		{
			return { ErrorCode::EC_TYPE_NOT_FOUND, fmt::format("Unable to load type database: {}", typeNotFoundException.what()) };
		}
		catch (const std::exception &somethingGoesWrong)
		{
			return { ErrorCode::EC_UNKNOWN_ERROR, fmt::format("Failed to load type database: {}", somethingGoesWrong.what()) };
		}

		std::terminate();
	}
}