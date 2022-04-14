#pragma once

#include <GameLib/IO/AssetKind.h>
#include <string>
#include <memory>


namespace gamelib::io
{
	class IOLevelAssetsProvider
	{
	public:
		virtual ~IOLevelAssetsProvider() noexcept = default;

		[[nodiscard]] virtual const std::string &getLevelName() const = 0;
		[[nodiscard]] virtual std::unique_ptr<uint8_t[]> getAsset(AssetKind kind, int64_t &bufferSize) const = 0;
		[[nodiscard]] virtual bool hasAssetOfKind(AssetKind kind) const = 0;
		[[nodiscard]] virtual bool isValid() const = 0;
	};
}