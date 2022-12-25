#pragma once

#include <filesystem>
#include <string>


namespace gtil
{
	enum class ErrorCode : int
	{
		EC_NO_ERROR = 0,
		EC_FAILED_TO_OPEN_INDEX = 1,
		EC_INVALID_INDEX_JSON,
		EC_INVALID_INDEX_SCHEMA,
		EC_UNABLE_TO_OPEN_TYPE_ENTRY,
		EC_INVALID_TYPE_ENTRY_JSON,
		EC_TYPE_NOT_FOUND,
		EC_UNKNOWN_ERROR
	};

	struct ErrorDescription
	{
		ErrorCode code { ErrorCode::EC_NO_ERROR };
		std::string message {};

		ErrorDescription() = default;
		ErrorDescription(ErrorCode c, const std::string &msg) : code(c), message(msg) {}

		explicit operator bool() const noexcept { return code == ErrorCode::EC_NO_ERROR; }

		static const ErrorDescription kNoError;
	};

	struct GlacierTypeInfoLoader
	{
		static ErrorDescription loadTypes(const std::filesystem::path &typesIndex);
	};
}