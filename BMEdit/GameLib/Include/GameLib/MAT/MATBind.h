#pragma once

#include <vector>

#include <GameLib/MAT/MATSprite.h>
#include <GameLib/MAT/MATOption.h>
#include <GameLib/MAT/MATTexture.h>
#include <GameLib/MAT/MATRenderState.h>
#include <GameLib/MAT/MATColorChannel.h>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	struct MATBind
	{
		MATBind();

		static MATBind makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		// Collected things
		std::string name {};  /// I assume, that we able to have no name or only 1 name...
		std::vector<MATRenderState> renderStates;
		std::vector<MATTexture> textures;
		std::vector<MATColorChannel> colorChannels;
		std::vector<MATSprite> sprites;
		/// ... another fields?
	};
}