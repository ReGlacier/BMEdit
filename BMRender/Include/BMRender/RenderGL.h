#pragma once


namespace bmr
{
	struct RenderGL
	{
		static void verifyState(const char *fileName, int line);
	};
}


#if !defined(NDEBUG)
	#define BM_RENDER_GL_CHECK_STATE() bmr::RenderGL::verifyState(__FILE__, __LINE__);
#else
	#define BM_RENDER_GL_CHECK_STATE()
#endif
