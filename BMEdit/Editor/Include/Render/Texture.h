#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <GameLib/TEX/TEXEntry.h>
#include <Render/GLResource.h>
#include <optional>
#include <cstdint>
#include <string>


namespace render
{
	struct Texture
	{
		uint16_t width { 0 };
		uint16_t height { 0 };
		GLuint texture { kInvalidResource };
		std::optional<std::uint32_t> index {}; /// Index of texture from TEX container
		std::optional<std::string> texPath {}; /// [Optional] Path to texture in TEX container (path may not be defined in TEX!)

		Texture();

		bool setup(QOpenGLFunctions_3_3_Core* gapi, const gamelib::tex::TEXEntry& gTex);

		void discard(QOpenGLFunctions_3_3_Core* gapi);

		void bind(QOpenGLFunctions_3_3_Core* gapi);

		void unbind(QOpenGLFunctions_3_3_Core* gapi);
	};
}