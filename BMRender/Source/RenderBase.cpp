#include <BMRender/RenderBase.h>
#include <algorithm>


namespace bmr
{
	void RenderBase::setup(RenderOptions &&options)
	{
		m_options = options;
		onSetupFinished();
	}

	void RenderBase::onSizeChanged(int newWidth, int newHeight)
	{
		if (m_options.width != newWidth || m_options.height != newHeight)
		{
			m_options.width = newWidth;
			m_options.height = newHeight;

			onSizeChangedImpl(newWidth, newHeight);
		}
	}

	void RenderBase::drawFrame()
	{
		clear();
		onBeginFrame();
		onEndFrame();
	}

	void RenderBase::onSizeChangedImpl(int width, int height)
	{
	}

	void RenderBase::onSetupFinished()
	{
	}

	void RenderBase::onBeginFrame()
	{
	}

	void RenderBase::onEndFrame()
	{
	}

	void RenderBase::clear()
	{
	}

	const RenderOptions &RenderBase::getOptions() const
	{
		return m_options;
	}

	RenderOptions &RenderBase::getOptions()
	{
		return m_options;
	}
}