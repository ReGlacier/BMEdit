#pragma once


namespace render
{
	struct ShaderConstants
	{
		static constexpr const char* kModelTransform = "i_uTransform.model";
		static constexpr const char* kCameraProjection = "i_uCamera.proj";
		static constexpr const char* kCameraView = "i_uCamera.view";
		static constexpr const char* kCameraResolution = "i_uCamera.resolution";
		static constexpr const char* kColor = "i_Color";
	};
}