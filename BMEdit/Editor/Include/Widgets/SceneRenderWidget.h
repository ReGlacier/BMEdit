#pragma once

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QString>

#include <Render/RenderEntry.h>

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


class QOpenGLFunctions_3_3_Core;

namespace widgets
{
	using RenderModeFlags = uint8_t;

	enum RenderMode : RenderModeFlags
	{
		RM_TEXTURE = 1 << 0,   ///< Render objects with textures
		RM_WIREFRAME = 1 << 1, ///< Render object in wireframe mode

		RM_NON_ALPHA_OBJECTS = 1 << 3, ///< Render only non transparent objects
		RM_ALPHA_OBJECTS = 1 << 4, ///< Render only transparent objects

		// Common
		RM_ALL = RM_TEXTURE | RM_WIREFRAME | RM_NON_ALPHA_OBJECTS | RM_ALPHA_OBJECTS,  ///< Render anything
		RM_DEFAULT = RM_TEXTURE | RM_NON_ALPHA_OBJECTS | RM_ALPHA_OBJECTS,  ///< Render in texture mode with alpha/non-alpha objects
	};

	struct RenderStats
	{
		QString currentRoom {};
		int allowedObjects { 0 };
		int rejectedObjects { 0 };
		float fFrameTime { .0f };  // how much time used for render this frame
	};

	enum class EObjectPriority
	{
		EP_STATIC_OBJECT = 0,
		EP_DYNAMIC_OBJECT = 1
	};

	struct RayCastObjectDescription
	{
		EObjectPriority ePrio { EObjectPriority::EP_STATIC_OBJECT };
		float fRayOriginDistance { .0f };
		gamelib::scene::SceneObject::Ptr pObject { nullptr };

		// Operators
		bool operator<(const RayCastObjectDescription& another) const;
	};

	struct SeebleObject
	{
		EObjectPriority ePrio { EObjectPriority::EP_STATIC_OBJECT };
		gamelib::scene::SceneObject::Ptr pObject {};
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

		[[nodiscard]] float getFOV() const { return m_camera.getFOV(); }
		void setFOV(float fov) { m_camera.setFOV(fov); }

		void setGeomViewMode(gamelib::scene::SceneObject* sceneObject);
		void setWorldViewMode();
		void resetViewMode();

		void setSelectedObject(gamelib::scene::SceneObject* sceneObject);
		void resetSelectedObject();

		[[nodiscard]] RenderModeFlags getRenderMode() const;
		void setRenderMode(RenderModeFlags renderMode);
		void resetRenderMode();

		void moveCameraTo(const glm::vec3& position);

		void reloadTexture(uint32_t textureIndex);

		bool shouldRenderPortals() const;
		void setShouldRenderPortals(bool bVal);

		bool shouldRenderRoomBoundingBox() const;
		void setShouldRenderRoomBoundingBox(bool bVal);

		int32_t getGameObjectPrimitiveId(const gamelib::scene::SceneObject::Ptr& pObject) const;
		int32_t getGameObjectPrimitiveId(const gamelib::scene::SceneObject* pObject) const;

		glm::mat4 getGameObjectTransform(const gamelib::scene::SceneObject::Ptr& pObject) const;
		glm::mat4 getGameObjectTransform(const gamelib::scene::SceneObject* pObject) const;

		std::optional<gamelib::BoundingBox> getGameObjectBoundingBox(const gamelib::scene::SceneObject::Ptr& pObject, bool bWorldTransform = true) const;
		std::optional<gamelib::BoundingBox> getGameObjectBoundingBox(const gamelib::scene::SceneObject* pObject, bool bWorldTransform = true) const;

		std::vector<RayCastObjectDescription> performRayCastToScene(const QPointF& screenSpace, const gamelib::scene::SceneObject::Ptr& pStartObject = nullptr) const;

	signals:
		void resourcesReady();
		void resourceLoadFailed(const QString& reason);
		void frameReady(const RenderStats& stats);

		void worldSelectionChanged(const std::vector<RayCastObjectDescription>& selectedObjects);

	public slots:
		void onRedrawRequested();

		// Use when object properties changed and his 'world transform' could be changed.
		void onObjectMoved(gamelib::scene::SceneObject* sceneObject);

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
		void doLoadTextures(QOpenGLFunctions_3_3_Core* glFunctions);
		void doLoadGeometry(QOpenGLFunctions_3_3_Core* glFunctions);
		void doCompileShaders(QOpenGLFunctions_3_3_Core* glFunctions);
		void doResetCameraState(QOpenGLFunctions_3_3_Core* glFunctions);
		void doPrepareInvalidatedResources(QOpenGLFunctions_3_3_Core* glFunctions);
		[[nodiscard]] glm::ivec2 getViewportSize() const;

		void collectRenderList(const render::Camera& camera, const gamelib::scene::SceneObject* pRootGeom, render::RenderEntriesList& entries, RenderStats& stats, bool bIgnoreVisibility);
		void collectRenderEntriesIntoRenderList(const gamelib::scene::SceneObject* pRootGeom, render::RenderEntriesList& entries, RenderStats& stats, bool bIgnoreVisibility, bool bBreakOnChild = false);
		void performRender(QOpenGLFunctions_3_3_Core* glFunctions, const render::RenderEntriesList& entries, const render::Camera& camera, const std::function<bool(const render::RenderEntry&)>& filter);

		void invalidateRenderList();

		void buildRoomCache(QOpenGLFunctions_3_3_Core* glFunctions);

		/**
		 * @brief Method trying to find a new room for current camera (if camera not in that room of bRejectLastResult is true)
		 * @param stats - reference to render stats object (method updates room name if new room presented)
		 * @param bRejectLastResult - pass true to reject current room and try to find a new one
		 */
		void updateCameraRoomAttachment(RenderStats& stats, bool bRejectLastResult = true);

	private:
		void beginDebugGroup(std::string_view groupName);
		void endDebugGroup();

	private:
		// Data
		gamelib::Level* m_pLevel { nullptr };

		// Camera & world view
		render::Camera m_camera {};
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
		bool m_bRenderPortals { false }; // Should we render portals between rooms (debug view)
		bool m_bRenderRoomBoundingBox { false }; // Should we render room bounding box (of all rooms)

		// View mode
		enum class EViewMode : uint8_t
		{
			VM_WORLD_VIEW,
			VM_GEOM_PREVIEW
		};

		EViewMode m_eViewMode { EViewMode::VM_WORLD_VIEW };
		gamelib::scene::SceneObject* m_pSceneObjectToView {};
		gamelib::scene::SceneObject* m_pSelectedSceneObject { nullptr };

		render::RenderEntriesList m_renderList {};

		struct GLResources;
		std::unique_ptr<GLResources> m_resources;

		struct RoomDef
		{
			enum class ELocation : int {
				eUNDEFINED = 0,
				eOUTSIDE = 1,
				eINSIDE = 2,
				eBOTH = 3,
			};

			enum class EBoundingBoxSource : int {
				BBS_ROOM_COLLISION_MESH,  ///< Calculated via collision mesh
				BBS_ZBOUNDS_AUTO_EXPAND,  ///< Calculated by ZBOUND object points
				BBS_AUTO_ROOM_EXPAND,     ///< Calculated as expand of all objects in room
				BBS_NONE                  ///< Not calculated or other BoundingBox source (auto-gen as example)
			};

			/**
			 * @brief Weak pointer to entity which represent room
			 */
			gamelib::scene::SceneObject::Ref rRoom {};

			/**
			 * @brief World space bounding box which cover whole room. Typically it's been built from collision box, but sometimes it could be a expanded bbox (expanded by children objects)
			 */
			gamelib::BoundingBox vBoundingBox {};

			/**
			 * @brief Type of room location. Seee ELocation.json for details
			 */
			ELocation eLocation { ELocation::eUNDEFINED };

			/**
			 * @brief How bounding box calculated
			 */
			EBoundingBoxSource eBoundingBoxSource { EBoundingBoxSource::BBS_NONE };

			/**
			 * @brief Information about room exits
			 */
			std::vector<gamelib::gms::room::ZRoomExit> aExists {};

			/**
			 * @brief Information about neighbour rooms
			 */
			std::vector<gamelib::gms::room::ZRoomNeighbor> aNeighbours {};

			/**
			 * @brief Room eXit geom boxes
			 */
			std::unique_ptr<render::Model> mExitsDebugModel { nullptr };

			/**
			 * @brief This list contains objects which could be visible in this specific room
			 */
			std::vector<SeebleObject> vObjects {};

			/**
			 * @brief Room bounding box debug model
			 */
			std::unique_ptr<render::Model> mBBoxModel { nullptr };

			/**
			 * @brief Means "is this room created because no other rooms exists"
			 */
			bool bIsVirtualBigRoom { false };
		};

		std::list<RoomDef> m_rooms {};
		std::list<const RoomDef*> m_cameraInRooms {};

	private:
		void computeRoomBoundingBox(RoomDef& d);
	};
}
