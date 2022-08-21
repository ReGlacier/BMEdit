#pragma once

#include <GameLib/IO/AssetKind.h>
#include <GameLib/Span.h>
#include <string>
#include <memory>


namespace gamelib::io
{
	class IOLevelAssetsProvider
	{
	public:
		virtual ~IOLevelAssetsProvider() noexcept = default;

		// Read API
		[[nodiscard]] virtual const std::string &getLevelName() const = 0;
		[[nodiscard]] virtual std::unique_ptr<uint8_t[]> getAsset(AssetKind kind, int64_t &bufferSize) const = 0;
		[[nodiscard]] virtual bool hasAssetOfKind(AssetKind kind) const = 0;

		// Write API
		virtual bool saveAsset(AssetKind kind, Span<uint8_t> assetBody) = 0;

		// Etc
		[[nodiscard]] virtual bool isValid() const = 0;
		[[nodiscard]] virtual bool isEditable() const = 0;
	};
}