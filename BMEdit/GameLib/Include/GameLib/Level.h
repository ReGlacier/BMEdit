#pragma once

#include <GameLib/IO/IOLevelAssetsProvider.h>
#include <memory>


namespace gamelib
{
	class Level
	{
	public:
		explicit Level(std::unique_ptr<io::IOLevelAssetsProvider> &&levelAssetsProvider);

		[[nodiscard]] const std::string &getLevelName() const;
	private:
		std::unique_ptr<io::IOLevelAssetsProvider> m_assetProvider;
	};
}