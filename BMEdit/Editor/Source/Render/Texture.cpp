#include <Render/Texture.h>


namespace render
{
	Texture::Texture() = default;

	void Texture::discard(QOpenGLFunctions_3_3_Core *gapi)
	{
		width = height = 0;

		if (texture != kInvalidResource)
		{
			gapi->glDeleteTextures(1, &texture);
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