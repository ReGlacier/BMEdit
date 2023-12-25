#include <Editor/TextureProcessor.h>
#include <Render/Texture.h>


namespace render
{
	Texture::Texture() = default;

	bool Texture::setup(QOpenGLFunctions_3_3_Core* gapi, const gamelib::tex::TEXEntry& gTex)
	{
		if (texture != kInvalidResource)
		{
			assert(false && "Resource must be not initialised here!");
			return false;
		}

		uint16_t w { 0 }, h { 0 };
		std::unique_ptr<std::uint8_t[]> decompressedMemBlk = editor::TextureProcessor::decompressRGBA(gTex, w, h, 0); //
		if (!decompressedMemBlk)
		{
			return false;
		}

		// Store texture index from TEX container
		index = std::make_optional(gTex.m_index);
		texPath = gTex.m_fileName;  // just copy file name from tex (if it defined!)
		width = w;
		height = h;

		// Create GL resource
		gapi->glGenTextures(1, &texture);
		gapi->glBindTexture(GL_TEXTURE_2D, texture);
		gapi->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, decompressedMemBlk.get());

		gapi->glGenerateMipmap(GL_TEXTURE_2D);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		gapi->glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	void Texture::discard(QOpenGLFunctions_3_3_Core *gapi)
	{
		width = height = 0;

		if (texture != kInvalidResource)
		{
			gapi->glDeleteTextures(1, &texture);
			texture = kInvalidResource;
		}
	}

	void Texture::bind(QOpenGLFunctions_3_3_Core* gapi)
	{
		if (texture != kInvalidResource)
		{
			gapi->glBindTexture(GL_TEXTURE_2D, texture);
		}
	}

	void Texture::unbind(QOpenGLFunctions_3_3_Core* gapi)
	{
		gapi->glBindTexture(GL_TEXTURE_2D, 0);
	}
}