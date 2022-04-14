#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>

extern "C" {
#include <unzip.h>
}


namespace editor
{
	class ZIPLevelAssetProvider : public gamelib::io::IOLevelAssetsProvider
	{
	public:
		explicit ZIPLevelAssetProvider(std::string containerPath);
		~ZIPLevelAssetProvider() override;

		[[nodiscard]] std::string getLevelName() const override;
		[[nodiscard]] std::unique_ptr<uint8_t[]> getAsset(gamelib::io::AssetKind kind, int64_t &bufferSize) const override;

	private:
		std::string m_path {};
		void *m_zipHandle { nullptr };
		mutable std::string m_levelName;
	};
}