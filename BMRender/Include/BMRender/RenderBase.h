#pragma once

#include <BMRender/RenderOptions.h>


namespace bmr
{
	class RenderBase
	{
	public:
		virtual ~RenderBase() noexcept = default;

		// Setup & loop
		void setup(RenderOptions&& options);
		void drawFrame();

		// Public callbacks
		void onSizeChanged(int newWidth, int newHeight);

	protected: // Self callbacks
		virtual void onSetupFinished();
		virtual void onSizeChangedImpl(int width, int height);
		virtual void onBeginFrame();
		virtual void onEndFrame();

	protected:
		void clear();
		[[nodiscard]] const RenderOptions& getOptions() const;
		[[nodiscard]] RenderOptions& getOptions();

	private:
		RenderOptions m_options {};
	};
}