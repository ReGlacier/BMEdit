#pragma once

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QString>
#include <QFile>

#include <Render/RenderEntry.h>
#include <Render/GizmoRenderer.h>

#include <GameLib/BoundingBox.h>
#include <GameLib/Level.h>
#include <GameLib/GMS/Room/ZRoomDefs.h>
#include <Render/Camera.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <list>
#include <string>


class QOpenGLFunctions_3_3_Core;

namespace widgets
{
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

		[[nodiscard]] float getFOV() const { return m_camera.getFOV(); }
		void setFOV(float fov) { m_camera.setFOV(fov); }

		void setGeomViewMode(gamelib::scene::SceneObject* sceneObject);
		void setWorldViewMode();
		void resetViewMode();

		void setSelectedObject(gamelib::scene::SceneObject* sceneObject);
		void resetSelectedObject();

		void moveCameraTo(const glm::vec3& position);

		bool shouldRenderPortals() const;
		void setShouldRenderPortals(bool bVal);

		bool shouldRenderRoomBoundingBox() const;
		void setShouldRenderRoomBoundingBox(bool bVal);

		void addGizmoLine(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color) { m_gizmo.addLine(a, b, color); }
		void addGizmoBox(const gamelib::BoundingBox &box, const glm::vec4 &color, const glm::vec4 &lineColor) { m_gizmo.addAABB(box, color, lineColor); }
		void addGizmoText(const std::string& text, const glm::vec2& pos, float size) { m_gizmo.addText(text, pos, size); }
		void clearGizmos() { m_gizmo.clear(); }
		bool setGizmoFont(QFile &file, int pixelSize);
	signals:
		void resourcesReady();
		void resourceLoadFailed(const QString& reason);

	public slots:
		void onRedrawRequested();

		// Use when object properties changed and his 'world transform' could be changed.
		void onObjectMoved(const QString &propertyName, gamelib::scene::SceneObject *sceneObject);

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
		bool checkRenderRequirements() const;
		void loadLevelImpl();
		void updateViewLists();
		void generateDrawCommands();
		void drawScene();
		void drawGizmo();
		void generateGizmosForEntity(gamelib::scene::SceneObject* pSceneObject);

		[[nodiscard]] glm::ivec2 getViewportSize() const {
			return { widthMM(), heightMM() };
		}

	private:
		// Data
		gamelib::Level* m_pLevel { nullptr };
		gamelib::scene::SceneObject* m_pSelectedObject{nullptr};

		// Render data
		struct RenderContext;
		std::unique_ptr<RenderContext> m_pContext { nullptr };

		struct RenderCommon;
		std::unique_ptr<RenderCommon> m_pCommon { nullptr };

		// Loader state
		enum class ELevelLoadState { LLS_NONE, LLS_LOADING, LLS_FAILED_TO_LOAD, LLS_READY };
		ELevelLoadState m_eLoaderState { ELevelLoadState::LLS_NONE };

		// Camera & world view
		render::Camera m_camera {};
		QPoint m_mouseLastPosition {};
		bool m_bFirstMouseQuery { true };
		bool m_bRenderPortals { false }; // Should we render portals between rooms (debug view)
		bool m_bRenderRoomBoundingBox { false }; // Should we render room bounding box (of all rooms)
		bool m_bRenderListDirty { false };
		bool m_bIgnoreVisibility { false };
		bool m_bTransformsDirty { false };

		render::GizmoRenderer m_gizmo;
	};
}
