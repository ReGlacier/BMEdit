#pragma once

#include <BMRender/RenderBase.h>
#include <BMRender/ShaderProgram.h>
#include <BMRender/Camera.h>
#include <BMRender/Mesh.h>
#include <GameLib/Level.h>
#include <cstdint>
#include <memory>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>


namespace bmr
{
	class SimplePrimitiveRender final : public RenderBase
	{
	public:
		explicit SimplePrimitiveRender(const gamelib::Level *level);

		// Primitive
		void setPrimitiveIndex(std::uint32_t primId);
		void resetPrimitiveIndex();
		[[nodiscard]] std::uint32_t getPrimitiveIndex() const;

		// Camera
		void setWireframeRenderEnabled(bool isEnabled);
		void setBoundingBoxRenderingEnabled(bool isEnabled);

		[[nodiscard]] Camera &getCamera();
		[[nodiscard]] const Camera &getCamera() const;
		[[nodiscard]] bool isWireframeRenderingEnabled() const;
		[[nodiscard]] bool isBoundingBoxRenderingEnabled() const;

	protected:
		void onSetupFinished() override;
		void onSizeChangedImpl(int width, int height) override;
		void onBeginFrame() override;
		void onEndFrame() override;

	private:
		void setupPrimitive();

	private:
		const gamelib::Level *m_level { nullptr };
		std::uint32_t m_primitiveIndex { 0u };
		bool m_primitiveChanged{ false };

		// Camera
		Camera m_camera {};

		// Material
		std::unique_ptr<ShaderProgram> m_primaryMaterial;

		// Mesh
		std::unique_ptr<Mesh> m_mesh { nullptr };
		std::unique_ptr<Mesh> m_boundingBox { nullptr };
		glm::mat4 m_meshModelMatrix { 1.f };
	};
}