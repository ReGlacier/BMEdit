#include <glad/glad.h>
#include <fmt/format.h>
#include <BMRender/RenderGL.h>
#include <BMRender/Logger.h>
#include <string>


namespace bmr
{
	void RenderGL::verifyState(const char *fileName, int line)
	{
#if !defined(NDEBUG)
		GLenum errorCode = glGetError();

		if (errorCode != GL_NO_ERROR)
		{
			std::string error;

#define BM_CASE_ERROR(err) case (err): error = #err; break;

			switch (errorCode)
			{
				BM_CASE_ERROR(GL_INVALID_ENUM)
				BM_CASE_ERROR(GL_INVALID_VALUE)
				BM_CASE_ERROR(GL_INVALID_OPERATION)
				BM_CASE_ERROR(GL_OUT_OF_MEMORY)
				BM_CASE_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION)
#ifdef GL_STACK_OVERFLOW
				BM_CASE_ERROR(GL_STACK_OVERFLOW)
#endif

#ifdef GL_STACK_UNDERFLOW
				BM_CASE_ERROR(GL_STACK_UNDERFLOW)
#endif
				default: error = fmt::format("Unknown GL error code 0x{:08X}", errorCode); break;
			}

			Logger::logMessage(Level::LL_ERROR, fmt::format("OpenGL Error (file {} at line {}): {}", fileName, line, error));
			assert(false);
		}

#undef BM_CASE_ERROR
#endif
	}
}