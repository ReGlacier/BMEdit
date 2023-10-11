#pragma once

#include <QOpenGLWidget>
#include <GameLib/Level.h>
#include <Render/Camera.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>


namespace renderer
{

}

class QOpenGLFunctions_3_3_Core;

namespace widgets
{
	using RenderModeFlags = uint8_t;

	enum RenderMode : RenderModeFlags
	{
		RM_TEXTURE = 1 << 0,
		RM_WIREFRAME = 1 << 1,

		RM_ALL = RM_TEXTURE | RM_WIREFRAME,
		RM_DEFAULT = RM_TEXTURE
	};

	class SceneRenderWidget : public QOpenGLWidget
	{
		Q_OBJECT
	public:
		SceneRenderWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
		~SceneRenderWidget() noexcept override;

		void setLevel(gamelib::Level* pLevel);
		void resetLevel();

		[[nodiscard]] render::Camera& getCamera() { return m_camera; }
		[[nodiscard]] const render::Camera& getCamera() const { return m_camera; }

		[[nodiscard]] float getFOV() const { return m_fFOV; }
		void setFOV(float fov) { m_fFOV = fov; m_bDirtyProj = true; }

		void setGeomViewMode(gamelib::scene::SceneObject* sceneObject);
		void setWorldViewMode();
		void resetViewMode();

		[[nodiscard]] RenderModeFlags getRenderMode() const;
		void setRenderMode(RenderModeFlags renderMode);
		void resetRenderMode();

		void moveCameraTo(const glm::vec3& position);

	signals:
		void resourcesReady();
		void resourceLoadFailed(const QString& reason);

	public slots:
		void onRedrawRequested();

	protected:
		void initializeGL() override;
		void paintGL() override;
		void resizeGL(int w, int h) override;

		void keyPressEvent(QKeyEvent *event) override;
		void mouseDoubleClickEvent(QMouseEvent *event) override;
		void mouseMoveEvent(QMouseEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;

	private:
		void updateProjectionMatrix(int w, int h);
		void doRenderScene(QOpenGLFunctions_3_3_Core* glFunctions);
		void doLoadTextures(QOpenGLFunctions_3_3_Core* glFunctions);
		void doLoadGeometry(QOpenGLFunctions_3_3_Core* glFunctions);
		void doCompileShaders(QOpenGLFunctions_3_3_Core* glFunctions);
		void doResetCameraState(QOpenGLFunctions_3_3_Core* glFunctions);
		void doRenderGeom(QOpenGLFunctions_3_3_Core* glFunctions, const gamelib::scene::SceneObject* geom, bool bIgnoreVisibility = false);

	private:
		// Data
		gamelib::Level* m_pLevel { nullptr };

		// Camera & world view
		render::Camera m_camera {};
		glm::mat4 m_matProjection {};
		float m_fFOV { 67.664f };
		float m_fZNear { .1f };
		float m_fZFar { 100'000.f };
		bool m_bDirtyProj { true };
		uint8_t m_renderMode = RenderMode::RM_DEFAULT;

		// State
		enum class ELevelState : uint8_t
		{
			LS_NONE = 0,
			LS_LOAD_TEXTURES = 1,
			LS_LOAD_GEOMETRY = 2,
			LS_COMPILE_SHADERS = 3,
			LS_RESET_CAMERA_STATE = 4,
			LS_READY
		};

		ELevelState m_eState { ELevelState::LS_NONE };
		QPoint m_mouseLastPosition {};
		bool m_bFirstMouseQuery { true };

		// View mode
		enum class EViewMode : uint8_t
		{
			VM_WORLD_VIEW,
			VM_GEOM_PREVIEW
		};

		EViewMode m_eViewMode { EViewMode::VM_WORLD_VIEW };
		gamelib::scene::SceneObject* m_pSceneObjectToView {};

		struct GLResources;
		std::unique_ptr<GLResources> m_resources;
	};
}
