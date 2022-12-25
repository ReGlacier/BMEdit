#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <GameLib/Span.h>
#include <unordered_map>
#include <memory>

namespace ap
{
	class ZIPLevelAssetProvider : public gamelib::io::IOLevelAssetsProvider
	{
	public:
		explicit ZIPLevelAssetProvider(std::string containerPath);
		~ZIPLevelAssetProvider() override;

		// Read API
		[[nodiscard]] const std::string &getLevelName() const override;
		[[nodiscard]] std::unique_ptr<uint8_t[]> getAsset(gamelib::io::AssetKind kind, int64_t &bufferSize) const override;
		[[nodiscard]] bool hasAssetOfKind(gamelib::io::AssetKind kind) const override;

		// Write API
		bool saveAsset(gamelib::io::AssetKind kind, gamelib::Span<uint8_t> assetBody) override;

		// Etc
		[[nodiscard]] bool isEditable() const override;
		[[nodiscard]] bool isValid() const override;

	private:
		std::string getAssetFileName(gamelib::io::AssetKind kind) const;

	private:
		struct Context;
		std::unique_ptr<Context> m_ctx { nullptr };
	};
}