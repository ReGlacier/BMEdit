#pragma once

#include <GameLib/IO/AssetKind.h>


namespace gamelib::io
{
	class IOLevelAssetsProvider
	{
	public:
		virtual ~IOLevelAssetsProvider() noexcept = default;

		virtual std::string getLevelName() const = 0;
		virtual std::unique_ptr<uint8_t[]> getAsset(AssetKind kind) const = 0;
	};
}