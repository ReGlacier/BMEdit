#include <Widgets/SceneRenderWidget.h>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLContext>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QFile>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/PRP/PRPObjectExtractor.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <GameLib/BoundingBox.h>
#include <GameLib/Plane.h>

#include <Render/ShaderConstants.h>
#include <Render/GlacierVertex.h>
#include <Render/GLResource.h>
#include <Render/Texture.h>
#include <Render/Shader.h>
#include <Render/Model.h>

#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <algorithm>
#include <chrono>
#include <set>


namespace widgets
{
	// Here stored geom names (common) where editor should avoid any rendering (it's too expensive and unnecessary for us)
	static const std::set<std::string_view> g_bannedObjectIds {
	    "AdditionalResources", "AllLevels/mainsceneincludes.zip", "AllLevels/equipment.zip"
	};

	bool RayCastObjectDescription::operator<(const widgets::RayCastObjectDescription& another) const
	{
		if (another.ePrio == ePrio)
		{
			if (std::fabsf(another.fRayOriginDistance - fRayOriginDistance) <= std::numeric_limits<float>::epsilon())
			{
				return false; // They are completely same (except object itself, but who cares?)
			}

			return fRayOriginDistance < another.fRayOriginDistance;
		}

		return static_cast<int>(ePrio) < static_cast<int>(another.ePrio);
	}

	using namespace render;

	struct SceneRenderWidget::GLResources
	{
		std::vector<Texture> m_textures {};
		std::vector<Shader> m_shaders {};
		std::vector<Model> m_models {};
		std::unordered_map<uint32_t, size_t> m_modelsCache {};  /// primitive index to model index in m_models
		std::unordered_map<gamelib::scene::SceneObject*, glm::mat4> m_modelTransformCache {}; /// transformations cache
		std::unordered_map<std::string, GLuint> m_textureNameToGL {}; /// name of texture to it's OpenGL resource id
		std::unordered_map<uint32_t, GLuint> m_textureIndexToGL {}; /// index of texture to it's OpenGL resource id
		std::unordered_set<uint32_t> m_invalidatedTextures; /// Set of textures who need to be reloaded on next frame
		GLuint m_iGLDebugTexture { 0 };
		GLuint m_iGLMissingTexture { 0 };
		GLuint m_iGLUnsupportedMaterialTexture { 0 };
		size_t m_iTexturedShaderIdx = 0;
		size_t m_iGizmoShaderIdx = 0;

		GLResources() {}
		~GLResources() {}

		void discard(QOpenGLFunctions_3_3_Core* gapi)
		{
			// Destroy textures
			{
				for (auto& texture : m_textures)
				{
					texture.discard(gapi);
				}

				m_textures.clear();
			}

			// Destroy shaders
			{
				for (auto& shader : m_shaders)
				{
					shader.discard(gapi);
				}

				m_shaders.clear();
			}

			// Destroy models
			{
				for (auto& model : m_models)
				{
					model.discard(gapi);
				}

				m_models.clear();
			}

			// Empty cache
			m_modelsCache.clear();
			m_modelTransformCache.clear();
			m_textureNameToGL.clear();
			m_textureIndexToGL.clear();
			m_invalidatedTextures.clear();

			// Release refs
			m_iGLDebugTexture = 0u;
			m_iGLMissingTexture = 0u;
			m_iGLUnsupportedMaterialTexture = 0u;
			m_iTexturedShaderIdx = 0u;
			m_iGizmoShaderIdx = 0u;
		}

		[[nodiscard]] bool hasResources() const
		{
			return !m_textures.empty() || !m_shaders.empty() || !m_models.empty();
		}
	};

	SceneRenderWidget::SceneRenderWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
	{
		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(3, 3);
		format.setProfile(QSurfaceFormat::CoreProfile);
		setFormat(format);
	}

	SceneRenderWidget::~SceneRenderWidget() noexcept = default;

	void SceneRenderWidget::paintGL()
	{
		RenderStats renderStats {};

		auto renderStartTime = std::chrono::high_resolution_clock::now();

		auto funcs = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
		if (!funcs) {
			qFatal("Could not obtain required OpenGL context version");
			return;
		}

		// Begin frame
		const auto vp = getViewportSize();
		funcs->glViewport(0, 0, vp.x, vp.y);
		funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		funcs->glClearColor(0.15f, 0.2f, 0.45f, 1.0f);

		// Z-Buffer testing
		funcs->glEnable(GL_DEPTH_TEST);

		// Blending
		funcs->glEnable(GL_BLEND);
		funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// NOTE: Before render anything we need to look at material and check MATRenderState.
		//       If it's applied we need to setup OpenGL into correct state to make perfect rendering
		switch (m_eState)
		{
			case ELevelState::LS_NONE:
			{
				if (m_pLevel) {
					// Create base for resources
					assert(m_resources == nullptr && "Leaked resources");
					m_resources = std::make_unique<GLResources>();

				    // Run process
				    m_eState = ELevelState::LS_LOAD_TEXTURES;
			    } else if (m_resources && m_resources->hasResources()) {
				    m_resources->discard(funcs);
			    }
		    }
		    break;
			case ELevelState::LS_LOAD_TEXTURES:
		    {
			    doLoadTextures(funcs);
		    }
		    break;
			case ELevelState::LS_LOAD_GEOMETRY:
		    {
			    doLoadGeometry(funcs);
		    }
		    break;
			case ELevelState::LS_COMPILE_SHADERS:
		    {
			    doCompileShaders(funcs);
		    }
		    break;
		    case ELevelState::LS_RESET_CAMERA_STATE:
		    {
			    doResetCameraState(funcs);
		    }
			break;
			case ELevelState::LS_READY:
		    {
			    // Prepare invalidated stuff
			    doPrepareInvalidatedResources(funcs);

			    // Render scene
			    gamelib::scene::SceneObject* pRoot = nullptr;
			    bool bIgnoreVisibility = false;

			    if (m_eViewMode == EViewMode::VM_WORLD_VIEW)
			    {
				    pRoot = m_pLevel->getSceneObjects()[0].get();
				}
			    else if (m_eViewMode == EViewMode::VM_GEOM_PREVIEW)
			    {
				    if (m_pSceneObjectToView)
				    {
						bIgnoreVisibility = true;
						pRoot = m_pSceneObjectToView;
					}
			    }

			    if (!pRoot)
				    return;

			    if (m_renderList.empty())
			    {
				    collectRenderList(m_camera, pRoot, m_renderList, renderStats, bIgnoreVisibility);
			    }

			    if (!m_renderList.empty())
			    {
				    auto onlyNonAlpha = [](const render::RenderEntry& entry) -> bool { return !entry.material.renderState.isAlphaTestEnabled() && !entry.material.renderState.isBlendEnabled(); };
				    auto onlyAlpha = [](const render::RenderEntry& entry) -> bool { return entry.material.renderState.isAlphaTestEnabled() || entry.material.renderState.isBlendEnabled(); };

				    // 2 pass rendering: first render only non-alpha objects
				    if (m_renderMode & RenderMode::RM_NON_ALPHA_OBJECTS)
				    {
					    performRender(funcs, m_renderList, m_camera, onlyNonAlpha);
				    }

				    // then render only alpha objects
				    if (m_renderMode & RenderMode::RM_ALPHA_OBJECTS)
				    {
					    performRender(funcs, m_renderList, m_camera, onlyAlpha);
				    }

				    // Submit stats
				    if (!m_renderList.empty())
				    {
					    auto renderEndTime = std::chrono::high_resolution_clock::now();
					    std::chrono::duration<float> elapsed = renderEndTime - renderStartTime;
					    renderStats.fFrameTime = elapsed.count();
					    emit frameReady(renderStats);
				    }
			    }
		    }
		    break;
		}
	}

	void SceneRenderWidget::resizeGL(int w, int h)
	{
		Q_UNUSED(w)
		Q_UNUSED(h)

		// Update projection
		m_camera.setViewport(w, h);

		// Because our list of visible objects could be changed here (???)
		invalidateRenderList();
	}

	void SceneRenderWidget::keyPressEvent(QKeyEvent* event)
	{
		if (m_pLevel)
		{
			render::CameraMovementMask movementMask {};

			if (event->key() == Qt::Key_W)
			{
				movementMask |= render::CameraMovementMaskValues::CM_FORWARD;
			}

			if (event->key() == Qt::Key_S)
			{
				movementMask |= render::CameraMovementMaskValues::CM_BACKWARD;
			}

			if (event->key() == Qt::Key_A)
			{
				movementMask |= render::CameraMovementMaskValues::CM_LEFT;
			}

			if (event->key() == Qt::Key_D)
			{
				movementMask |= render::CameraMovementMaskValues::CM_RIGHT;
			}

			if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
			{
				movementMask |= render::CameraMovementMaskValues::CM_SPEEDUP_MOD;
			}

			if ((movementMask & CM_FORWARD) && (movementMask & CM_BACKWARD)) movementMask &= ~(CM_FORWARD | CM_BACKWARD);
			if ((movementMask & CM_LEFT) && (movementMask & CM_RIGHT)) movementMask &= ~(CM_LEFT | CM_RIGHT);

			if (movementMask > 0 && movementMask != (CM_SPEEDUP_MOD))
			{
				m_camera.handleKeyboardMovement(movementMask /* dt */);

				invalidateRenderList();
				repaint();
			}
		}

		QOpenGLWidget::keyPressEvent(event);
	}

	void SceneRenderWidget::mouseDoubleClickEvent(QMouseEvent* event)
	{
		QOpenGLWidget::mouseDoubleClickEvent(event);
	}

	void SceneRenderWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (m_pLevel)
		{
			float xpos = static_cast<float>(event->pos().x());
			float ypos = static_cast<float>(event->pos().y());

			if (m_bFirstMouseQuery)
			{
				m_mouseLastPosition = event->pos();
				m_bFirstMouseQuery = false;
				return;
			}

			float xOffset = static_cast<float>(xpos - m_mouseLastPosition.x());
			float yOffset = static_cast<float>(m_mouseLastPosition.y() - ypos);

			m_mouseLastPosition = event->pos();

			// Update camera
			const float kMinMovement = 0.001f;
			if (std::fabsf(xOffset - kMinMovement) > std::numeric_limits<float>::epsilon() || std::fabsf(yOffset - kMinMovement) > std::numeric_limits<float>::epsilon())
			{
				invalidateRenderList();
				m_camera.processMouseMovement(xOffset, yOffset /* dt */);
			}
		}

		repaint();
		QOpenGLWidget::mouseMoveEvent(event);
	}

	void SceneRenderWidget::mousePressEvent(QMouseEvent* event)
	{
		QOpenGLWidget::mousePressEvent(event);

		if (event->button() == Qt::MouseButton::RightButton && !m_pLevel->getSceneObjects().empty())
		{
			// Begin ray cast
			const auto vMouseClickPos = event->position();

			// If no rooms on level we should use ROOT as initial point (not recommended in MOST cases)
			// Two step raycast: 1 - to room bbox (allow to run ray from room)
			//                   2 - to in-room objects

//			auto result = performRayCastToScene(vMouseClickPos, (!m_pLastRoom && m_rooms.empty()) ? m_pLevel->getSceneObjects()[0] : nullptr);
//			if (!result.empty())
//			{
//				emit worldSelectionChanged(result);
//			}
		}
	}

	void SceneRenderWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		QOpenGLWidget::mouseReleaseEvent(event);

		m_bFirstMouseQuery = true;
		m_mouseLastPosition = QPoint(0, 0);
	}

	void SceneRenderWidget::setLevel(gamelib::Level *pLevel)
	{
		if (m_pLevel != pLevel)
		{
			m_resources = nullptr;
			m_eState = ELevelState::LS_NONE;
			m_pLevel = pLevel;
			m_bFirstMouseQuery = true;
			invalidateRenderList();
			resetViewMode();
			resetRenderMode();
		}
	}

	void SceneRenderWidget::resetLevel()
	{
		if (m_pLevel != nullptr)
		{
			m_resources = nullptr;
			m_eState = ELevelState::LS_NONE;
			m_pLevel = nullptr;
			m_bFirstMouseQuery = true;
			invalidateRenderList();
			resetViewMode();
			resetRenderMode();
			repaint();
		}
	}

	void SceneRenderWidget::setGeomViewMode(gamelib::scene::SceneObject* sceneObject)
	{
		assert(sceneObject != nullptr);

		if (sceneObject != m_pSceneObjectToView)
		{
			m_eViewMode = EViewMode::VM_GEOM_PREVIEW;
			m_pSceneObjectToView = sceneObject;
			invalidateRenderList();
			repaint();
		}
	}

	void SceneRenderWidget::setWorldViewMode()
	{
		m_eViewMode = EViewMode::VM_WORLD_VIEW;
		m_pSceneObjectToView = nullptr;
		invalidateRenderList();
		repaint();
	}

	void SceneRenderWidget::resetViewMode()
	{
		setWorldViewMode();
	}

	void SceneRenderWidget::setSelectedObject(gamelib::scene::SceneObject* sceneObject)
	{
		if (!m_pLevel)
			return;

		auto flags = sceneObject->getGeomInfo().getGeomFlags();
		bool isBit4Set = flags & (1 << 4);

		if (m_pSelectedSceneObject != sceneObject && sceneObject != nullptr)
		{
			m_pSelectedSceneObject = sceneObject;
			invalidateRenderList();
			repaint();
		}
	}

	void SceneRenderWidget::resetSelectedObject()
	{
		if (m_pSelectedSceneObject != nullptr)
		{
			m_pSelectedSceneObject = nullptr;

			if (m_pLevel)
			{
				invalidateRenderList();
				repaint();
			}
		}
	}

	RenderModeFlags SceneRenderWidget::getRenderMode() const
	{
		return m_renderMode;
	}

	void SceneRenderWidget::setRenderMode(RenderModeFlags renderMode)
	{
		m_renderMode = renderMode;
		repaint();
	}

	void SceneRenderWidget::resetRenderMode()
	{
		m_renderMode = RenderMode::RM_DEFAULT;
		repaint();
	}

	void SceneRenderWidget::moveCameraTo(const glm::vec3& position)
	{
		if (!m_pLevel)
			return;

		m_camera.setPosition(position);
		repaint();
	}

	void SceneRenderWidget::reloadTexture(uint32_t textureIndex)
	{
		if (!m_pLevel)
			return;

		m_resources->m_invalidatedTextures.insert(textureIndex);
	}

	bool SceneRenderWidget::shouldRenderPortals() const
	{
		return m_bRenderPortals;
	}

	void SceneRenderWidget::setShouldRenderPortals(bool bVal)
	{
		if (m_bRenderPortals != bVal)
		{
			m_bRenderPortals = bVal;
			repaint();
		}
	}

	bool SceneRenderWidget::shouldRenderRoomBoundingBox() const
	{
		return m_bRenderRoomBoundingBox;
	}

	void SceneRenderWidget::setShouldRenderRoomBoundingBox(bool bVal)
	{
		if (m_bRenderRoomBoundingBox != bVal)
		{
			m_bRenderRoomBoundingBox = bVal;
			repaint();
		}
	}

	int32_t SceneRenderWidget::getGameObjectPrimitiveId(const gamelib::scene::SceneObject::Ptr& pObject) const
	{
		if (!pObject) return 0;

		return getGameObjectPrimitiveId(pObject.get());
	}

	int32_t SceneRenderWidget::getGameObjectPrimitiveId(const gamelib::scene::SceneObject* pObject) const
	{
		if (!pObject) return 0;

		if (pObject->isInheritedOf("ZItem")) // Need support of item ammo & item container here
		{
			//ZItems has no PrimId. Instead of this they are refs to another geom by path
			auto rItemTemplatePath = pObject->getProperties().getObject<std::string>("rItemTemplate");
			const auto pItemTemplate = m_pLevel->getSceneObjectByGEOMREF(rItemTemplatePath);

			if (pItemTemplate)
			{
				gamelib::scene::SceneObject::Ptr pItem = nullptr;

				// Item found by path. That's cool! But this is not an item, for item need to ask Ground... object inside
				for (const auto& childRef : pItemTemplate->getChildren())
				{
					if (auto child = childRef.lock(); child && child->getName().starts_with("Ground"))
					{
						pItem = child;
						break;
					}
				}

				if (pItem)
				{
					// Nice! Now we ready to replace primId
					return pItem->getProperties().getObject<std::int32_t>("PrimId");
				}
			}
		}

		return pObject->getProperties().getObject<int32_t>("PrimId", 0);
	}

	glm::mat4 SceneRenderWidget::getGameObjectTransform(const gamelib::scene::SceneObject::Ptr& pObject) const
	{
		if (!pObject) return glm::mat4(1.f);
		return getGameObjectTransform(pObject.get());
	}

	glm::mat4 SceneRenderWidget::getGameObjectTransform(const gamelib::scene::SceneObject* pObject) const
	{
		if (auto it = m_resources->m_modelTransformCache.find(const_cast<gamelib::scene::SceneObject*>(pObject)); it != m_resources->m_modelTransformCache.end())
		{
			return it->second;
		}
		else
		{
			glm::mat4 mWorldTransform = pObject->getWorldTransform();
			m_resources->m_modelTransformCache[const_cast<gamelib::scene::SceneObject*>(pObject)] = mWorldTransform;
			return mWorldTransform;
		}

		// idk)
		return glm::mat4(1.f);
	}

	std::optional<gamelib::BoundingBox> SceneRenderWidget::getGameObjectBoundingBox(const gamelib::scene::SceneObject::Ptr& pObject, bool bWorldTransform) const
	{
		if (!pObject) return std::nullopt;
		return getGameObjectBoundingBox(pObject.get(), bWorldTransform);
	}

	std::optional<gamelib::BoundingBox> SceneRenderWidget::getGameObjectBoundingBox(const gamelib::scene::SceneObject* pObject, bool bWorldTransform) const
	{
		if (!pObject) return std::nullopt;

		auto primId = getGameObjectPrimitiveId(pObject);
		if (primId == 0)
		{
			return std::nullopt;
		}

		const Model& model = m_resources->m_models[m_resources->m_modelsCache[primId]];
		if (bWorldTransform)
		{
			glm::mat4 mWorldTransform = getGameObjectTransform(pObject);
			return gamelib::BoundingBox::toWorld(model.boundingBox, mWorldTransform);
		}

		return model.boundingBox;
	}

	std::vector<RayCastObjectDescription> SceneRenderWidget::performRayCastToScene(const QPointF& screenSpace, const gamelib::scene::SceneObject::Ptr& pStartObject) const
	{
		render::Ray sRay = m_camera.getRayFromScreen(static_cast<float>(screenSpace.x()),
		                                             static_cast<float>(screenSpace.y()));
		std::vector<RayCastObjectDescription> collectedObjects {};

		gamelib::scene::SceneObject* pRoot = pStartObject.get();

		// Need to find intersects with this thing. Need to visit only current room
		if (pRoot)
		{
			using R = gamelib::scene::SceneObject::EVisitResult;

			static EObjectPriority s_CurrentPrio = EObjectPriority::EP_STATIC_OBJECT;

			auto hitObjVisitor = [&sRay, &collectedObjects, this](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (auto bbox = getGameObjectBoundingBox(pObject); bbox.has_value())
				{
					// Need to exclude objects where bbox origin is inside
					if (sRay.intersect(bbox.value(), false))
					{
						auto& obj = collectedObjects.emplace_back();
						obj.ePrio = s_CurrentPrio;
						obj.pObject = pObject;
						obj.fRayOriginDistance = glm::distance(sRay.vOrigin, pObject->getPosition());
						return R::VR_NEXT;
					}
				}

				return R::VR_CONTINUE;
			};

			// Hit dynamic (not implemented yet)
			s_CurrentPrio = EObjectPriority::EP_DYNAMIC_OBJECT; // Now dynamic objects
			// TODO: Iterate over dynamic objects and check collision with them

			// Hit static
			s_CurrentPrio = EObjectPriority::EP_STATIC_OBJECT; // Now static objects
			pRoot->visitChildren(hitObjVisitor);

			// Sort hit list by distance to camera
			std::sort(collectedObjects.begin(), collectedObjects.end());

			return collectedObjects;
		}

		return collectedObjects;
	}

	void SceneRenderWidget::onRedrawRequested()
	{
		if (m_pLevel)
			repaint();
	}

	void SceneRenderWidget::onObjectMoved(gamelib::scene::SceneObject* sceneObject)
	{
		if (!sceneObject || !m_pLevel || !m_resources)
			return;

		// Visit limited subtree
		int iDepth = 2;  // max 2 objects, otherwise it's better to make full invalidation (in case when user wants to move some huge object)
		sceneObject->visitChildren([this, &iDepth](const gamelib::scene::SceneObject::Ptr& pObject) -> gamelib::scene::SceneObject::EVisitResult {
			// Update transform
			m_resources->m_modelTransformCache[pObject.get()] = pObject->getWorldTransform();

			--iDepth;
			return iDepth > 0 ? gamelib::scene::SceneObject::EVisitResult::VR_CONTINUE  // Go deeper
			                  : gamelib::scene::SceneObject::EVisitResult::VR_STOP_ALL; // Out of limit
		});

		invalidateRenderList();  //TODO: Need invalidate only object, not whole list!
		repaint();
	}

#define LEVEL_SAFE_CHECK() \
		if (!m_pLevel) \
		{ \
			m_eState = ELevelState::LS_NONE; \
			if (m_resources) \
			{ \
				m_resources->discard(glFunctions); \
			} \
			return; \
		}

	void SceneRenderWidget::doLoadTextures(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// Do it at once
		// TODO: Optimize and load "chunk by chunk"
		for (const auto& texture : m_pLevel->getSceneTextures()->entries)
		{
			// TODO: Support mip levels here?
			if (texture.m_mipLevels.empty())
			{
				// create null texture
				m_resources->m_textures.emplace_back();
				qWarning() << "Failed to load texture #" << texture.m_index << ". Reason: no mip levels (empty texture)";
				continue;
			}

			// Ok, texture is ok - load it
			Texture newTexture {};

			if (!newTexture.setup(glFunctions, texture))
			{
				m_resources->m_textures.emplace_back();
				qWarning() << "Failed to load texture #" << texture.m_index << ". Reason: setup failed";
				continue;
			}

			// Precache debug texture if it's not precached yet
			static constexpr const char* kGlacierMissingTex = "_Glacier/Missing_01";
			static constexpr const char* kWorldColiTex = "_TEST/Worldcoli";

			if (m_resources->m_iGLDebugTexture == 0 && texture.m_fileName.has_value() && (texture.m_fileName.value() == kGlacierMissingTex || texture.m_fileName.value() == kWorldColiTex))
			{
				m_resources->m_iGLDebugTexture = newTexture.texture;
			}

			// Update cache
			if (newTexture.texPath.has_value())
			{
				m_resources->m_textureNameToGL[newTexture.texPath.value()] = newTexture.texture;
			}

			if (newTexture.index.has_value())
			{
				m_resources->m_textureIndexToGL[newTexture.index.value()] = newTexture.texture;
			}

			// Save texture
			m_resources->m_textures.emplace_back(newTexture);
		}

		// And load extra textures (render specific)
		auto uploadQImageToGPU = [](QOpenGLFunctions_3_3_Core* gapi, const QImage& image) -> GLuint
		{
			GLuint textureId;
			gapi->glGenTextures(1, &textureId);
			gapi->glBindTexture(GL_TEXTURE_2D, textureId);
			gapi->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());

			gapi->glGenerateMipmap(GL_TEXTURE_2D);
			gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			gapi->glBindTexture(GL_TEXTURE_2D, 0);

			return textureId;
		};

		{
			QImage missingTextureImage = QImage(":/bmedit/mtl_missing_texture.png").convertToFormat(QImage::Format_RGBA8888, Qt::AutoColor);

			auto& missingTexture = m_resources->m_textures.emplace_back();
			missingTexture.texture = uploadQImageToGPU(glFunctions, missingTextureImage);
			missingTexture.width = missingTextureImage.width();
			missingTexture.height = missingTextureImage.height();

			m_resources->m_iGLMissingTexture = missingTexture.texture;
		}

		{
			QImage unsupportedMaterialTextureImage = QImage(":/bmedit/mtl_unsupported.png").convertToFormat(QImage::Format_RGBA8888, Qt::AutoColor);

			auto& unsupportedMaterial = m_resources->m_textures.emplace_back();
			unsupportedMaterial.texture = uploadQImageToGPU(glFunctions, unsupportedMaterialTextureImage);
			unsupportedMaterial.width = unsupportedMaterialTextureImage.width();
			unsupportedMaterial.height = unsupportedMaterialTextureImage.height();

			m_resources->m_iGLUnsupportedMaterialTexture = unsupportedMaterial.texture;
		}

		// It's done
		qDebug() << "All textures (" << m_pLevel->getSceneTextures()->entries.size() << ") are loaded and ready to be used";
		m_eState = ELevelState::LS_LOAD_GEOMETRY;
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doLoadGeometry(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// TODO: Optimize and load "chunk by chunk"
		for (const auto& model : m_pLevel->getLevelGeometry()->primitives.models)
		{
			if (model.meshes.empty())
			{
				// create null model
				m_resources->m_models.emplace_back();
				qWarning() << "Failed to load model of chunk " << model.chunk << ". Reason: no meshes (empty model)";
				continue;
			}

			Model& glModel = m_resources->m_models.emplace_back();
			glModel.chunkId = model.chunk;
			glModel.boundingBox = gamelib::BoundingBox(model.boundingBox.vMin, model.boundingBox.vMax);

			// And create mesh for bounding box
			glModel.setupBoundingBox(glFunctions);

			// Store cache
			m_resources->m_modelsCache[model.chunk] = m_resources->m_models.size() - 1;

			// Lookup mesh
			int meshIdx = 0;
			for (const auto& mesh : model.meshes)
			{
				if (mesh.vertices.empty())
				{
					// create empty mesh
					glModel.meshes.emplace_back();
					qWarning() << "Failed to load mesh #" << meshIdx << " of model at chunk " << model.chunk << ". Reason: no meshes (empty model)";
					++meshIdx;
					continue;
				}

				// Convert vertices & indices to single memory chunk
				std::vector<GlacierVertex> vertices;
				std::vector<std::uint16_t> indices;

				vertices.resize(mesh.vertices.size());
				indices.reserve(mesh.indices.size() * 3); // each 'index' subject contains three values

				for (int i = 0; i < mesh.vertices.size(); i++)
				{
					vertices[i].vPos = mesh.vertices[i];

					if (mesh.uvs.empty())
					{
						vertices[i].vUV = glm::vec2(.0f); // TODO: Idk what I should do here...
					}
					else
					{
						vertices[i].vUV = mesh.uvs[i];
					}
				}

				for (const auto& [a,b,c] : mesh.indices)
				{
					indices.emplace_back(a);
					indices.emplace_back(b);
					indices.emplace_back(c);
				}

				// And upload it to GPU
				Mesh& glMesh = glModel.meshes.emplace_back();
				glMesh.trianglesCount = mesh.trianglesCount;
				glMesh.variationId = mesh.variationId;

				if (!glMesh.setup(glFunctions, GlacierVertex::g_FormatDescription, vertices, indices, false))
				{
					qWarning() << "Failed to upload mesh #" << meshIdx << " of model at chunk " << model.chunk << ". Reason: failed to upload resource to GPU!";
					++meshIdx;
					continue;
				}

				// Precache color texture
				glMesh.materialId = mesh.material_id;

				if (glMesh.materialId > 0)
				{
					// Use material (for meshes)
					// First of all we need to know that 'shadows' and other things must be filtered here
					const auto& instances = m_pLevel->getLevelMaterials()->materialInstances;
					const auto& classes = m_pLevel->getLevelMaterials()->materialClasses;
					const auto& matInstance = instances[mesh.material_id - 1];

					if (const auto& parentName = matInstance.getParentName(); parentName == "StaticShadow" || parentName == "StaticShadowTextureShadow" || matInstance.getName().find("AlwaysInShadow") != std::string::npos)
					{
						// Shadows - do not use texturing (and don't show for now)
						glMesh.glTextureId = kInvalidResource;
					}
					else if (parentName == "Bad")
					{
						// Use 'bad' debug texture
						glMesh.glTextureId = m_resources->m_iGLUnsupportedMaterialTexture;
					}
					else
					{
						bool bTextureFound = false;

						// Here we need to find 'color' texture. In most cases we able to use matDiffuse as color texture
						for (const auto& binder : matInstance.getBinders())
						{
							if (bTextureFound)
								break;

							for (const auto& texture : binder.textures)
							{
								if (texture.getName() == "mapDiffuse" && (texture.getTextureId() != 0 || !texture.getTexturePath().empty()))
								{
									// And find texture in textures pool
									for (const auto& textureResource : m_resources->m_textures)
									{
										switch (texture.getPresentedTextureSources())
										{
											case gamelib::mat::PresentedTextureSource::PTS_NOTHING:
											    continue;  // Nothing

										    case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_ID:
										    {
											    // Only texture id
											    if (textureResource.index.has_value() && textureResource.index.value() == texture.getTextureId())
											    {
												    // Good
												    glMesh.glTextureId = textureResource.texture;
												    bTextureFound = true;
												    break;
											    }
										    }
											break;
										    case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_PATH:
										    {
											    // Only path
											    if (textureResource.texPath.has_value() && textureResource.texPath.value() == texture.getTexturePath())
											    {
												    // Good
												    glMesh.glTextureId = textureResource.texture;
												    bTextureFound = true;
												    break;
											    }
										    }
											break;
										    default:
										    {
											    // Bad case! Undefined behaviour!
											    assert(false && "Impossible case!");
											    continue;
										    }
										}
									}

									if (!bTextureFound)
									{
										// Use error texture
										glMesh.glTextureId = m_resources->m_iGLMissingTexture;
									}

									// But mark us as 'found'
									bTextureFound = true;

									// Done
									break;
								}
							}
						}

						// For debug only
//						if (glMesh.glTextureId == kInvalidResource)
//						{
//							glMesh.glTextureId = m_resources->m_iGLMissingTexture;
//						}
					}
				}
				else if (mesh.textureId > 0)
				{
					// Use texture here (for sprites). Need to find that texture in loaded textures list
					for (const auto& texture : m_resources->m_textures)
					{
						if (texture.index.has_value() && texture.index.value() == mesh.textureId)
						{
							glMesh.glTextureId = texture.texture;
							break;
						}
					}
				}
				// Otherwise no texture. So, we will render only bounding box (if it needed)

				// Next mesh
				++meshIdx;
			}
		}

		// Then load rooms cache
		buildRoomCache(glFunctions);

		qDebug() << "All models (" << m_pLevel->getLevelGeometry()->primitives.models.size() << ") are loaded & ready to use!";
		m_eState = ELevelState::LS_COMPILE_SHADERS;
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doCompileShaders(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// Load shaders from resources
		QFile coloredEntityVertexShader(":/bmedit/mtl_colored_gl33.vsh");
		QFile coloredEntityFragmentShader(":/bmedit/mtl_colored_gl33.fsh");
		QFile texturedEntityVertexShader(":/bmedit/mtl_textured_gl33.vsh");
		QFile texturedEntityFragmentShader(":/bmedit/mtl_textured_gl33.fsh");

		coloredEntityVertexShader.open(QIODevice::ReadOnly);
		coloredEntityFragmentShader.open(QIODevice::ReadOnly);
		texturedEntityVertexShader.open(QIODevice::ReadOnly);
		texturedEntityFragmentShader.open(QIODevice::ReadOnly);

		const std::string texturedEntityVertexShaderSource = texturedEntityVertexShader.readAll().toStdString();
		const std::string texturedEntityFragmentShaderSource = texturedEntityFragmentShader.readAll().toStdString();
		const std::string coloredEntityVertexShaderSource = coloredEntityVertexShader.readAll().toStdString();
		const std::string coloredEntityFragmentShaderSource = coloredEntityFragmentShader.readAll().toStdString();

		if (texturedEntityVertexShaderSource.empty())
		{
			emit resourceLoadFailed(QString("Failed to compile shaders (textured:vertex): no embedded asset found."));
			return;
		}

		if (texturedEntityFragmentShaderSource.empty())
		{
			emit resourceLoadFailed(QString("Failed to compile shaders (textured:fragment): no embedded asset found."));
			return;
		}

		if (coloredEntityVertexShaderSource.empty())
		{
			emit resourceLoadFailed(QString("Failed to compile shaders (colored:vertex): no embedded asset found."));
			return;
		}

		if (coloredEntityFragmentShaderSource.empty())
		{
			emit resourceLoadFailed(QString("Failed to compile shaders (colored:fragment): no embedded asset found."));
			return;
		}

		// Compile shaders
		std::string compileError;
		{
			Shader texturedShader;

			if (!texturedShader.compile(glFunctions, texturedEntityVertexShaderSource, texturedEntityFragmentShaderSource, compileError))
			{
				m_pLevel = nullptr;
				m_eState = ELevelState::LS_NONE;

				emit resourceLoadFailed(QString("Failed to compile shaders (textured): %1").arg(QString::fromStdString(compileError)));
				return;
			}

			m_resources->m_shaders.emplace_back(texturedShader);
			m_resources->m_iTexturedShaderIdx = m_resources->m_shaders.size() - 1;
		}

		{
			Shader gizmoShader;
			if (!gizmoShader.compile(glFunctions, coloredEntityVertexShaderSource, coloredEntityFragmentShaderSource, compileError))
			{
				m_pLevel = nullptr;
				m_eState = ELevelState::LS_NONE;

				emit resourceLoadFailed(QString("Failed to compile shaders (colored): %1").arg(QString::fromStdString(compileError)));
				return;
			}

			m_resources->m_shaders.emplace_back(gizmoShader);
			m_resources->m_iGizmoShaderIdx = m_resources->m_shaders.size() - 1;
		}

		qDebug() << "Shaders (" << m_resources->m_shaders.size() << ") compiled and ready to use!";
		m_eState = ELevelState::LS_RESET_CAMERA_STATE;
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doResetCameraState(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// ----------------------------------------------------------
		// Ok, first of all let's try to find where located ZPlayer of ZHitman3 object
		gamelib::scene::SceneObject::Ptr player = nullptr;

		m_pLevel->forEachObjectOfType("ZHitman3", [&player](const gamelib::scene::SceneObject::Ptr& sceneObject) -> bool {
			player = sceneObject;
			return true;
		});

		if (player)
		{
			// Ok, level contains player. Let's take his room and move camera to player
			const auto iPrimId = getGameObjectPrimitiveId(player);
			const auto vPlayerPosition = player->getParent().lock()->getPosition();
			glm::vec3 vCameraPosition = vPlayerPosition;

			// In theory, we need to put camera around player, not in player. So we need to have bounding box of player to correct camera position
			if (iPrimId != 0 && m_resources->m_modelsCache.contains(iPrimId))
			{
				const auto& sBoundingBox = m_resources->m_models[m_resources->m_modelsCache[iPrimId]].boundingBox;
				glm::vec3 vCenter = sBoundingBox.getCenter();
				vCenter.y += 1.5f * vCenter.y;

				vCameraPosition += vCenter;
			}

			m_camera.setPosition(vCameraPosition);
			qDebug() << "Move camera to object " << QString::fromStdString(player->getName()) << " at (" << vCameraPosition.x << ';' << vCameraPosition.y << ';' <<  vCameraPosition.z << ")";
		}
		else
		{
			// Bad for us, player not found. Need to put camera somewhere else
			qDebug() << "No player on scene. Camera moved to (0;0;0)";
			m_camera.setPosition(glm::vec3(0.f));
		}

		// ----------------------------------------------------------
		emit resourcesReady();

		m_eState = ELevelState::LS_READY; // Done!
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doPrepareInvalidatedResources(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		if (!m_resources->m_invalidatedTextures.empty())
		{
			for (auto& texture : m_resources->m_textures)
			{
				if (texture.index.has_value() && m_resources->m_invalidatedTextures.contains(texture.index.value()))
				{
					const uint32_t textureIndex = texture.index.value();

					// Unload texture
					texture.discard(glFunctions);

					// Load texture (need to find actual entry in global textures pool... bruh)
					const auto& allTextures = m_pLevel->getSceneTextures()->entries;
					auto it = std::find_if(allTextures.begin(), allTextures.end(), [textureIndex](const gamelib::tex::TEXEntry& ent) -> bool {
						return ent.m_index == textureIndex;
					});

					if (it != allTextures.end())
					{
						// Erase cache
						if (it->m_fileName.has_value())
						{
							m_resources->m_textureNameToGL.erase(it->m_fileName.value());
						}
						m_resources->m_textureIndexToGL.erase(it->m_index);

						// Reload
						if (texture.setup(glFunctions, *it))
						{
							// Update cache
							if (texture.texPath.has_value())
							{
								m_resources->m_textureNameToGL[texture.texPath.value()] = texture.texture;
							}

							if (texture.index.has_value())
							{
								m_resources->m_textureIndexToGL[texture.index.value()] = texture.texture;
							}

							// Done
							qDebug() << "Texture #" << textureIndex << " reloaded!";
						}
						else
						{
							qWarning() << "Failed to update texture #" << textureIndex;
						}
					}

					// Validated
					m_resources->m_invalidatedTextures.erase(textureIndex);
				}
			}
		}
	}

	glm::ivec2 SceneRenderWidget::getViewportSize() const
	{
		return { QWidget::width(), QWidget::height() };
	}

	void SceneRenderWidget::collectRenderList(const render::Camera& camera, const gamelib::scene::SceneObject* pRootGeom, render::RenderEntriesList& entries, RenderStats& stats, bool bIgnoreVisibility)
	{
		if (!m_pLevel || m_pLevel->getSceneObjects().empty())
		{
			return;
		}

		// Update room
		updateCameraRoomAttachment(stats);

		if (pRootGeom != m_pLevel->getSceneObjects()[0].get())
		{
			// Render from specific node (no performance optimisations here)
			collectRenderEntriesIntoRenderList(pRootGeom, entries, stats, bIgnoreVisibility);
		}
		else
		{
			// Try to render
			std::set<const gamelib::scene::SceneObject*> visitedObjects {};

			for (const auto& pRoom : m_cameraInRooms)
			{
				for (const SeebleObject& sObject : pRoom->vObjects)
				{
					if (visitedObjects.contains(sObject.pObject.get()))
						continue; // Skip because it's in render list already

					if (auto bbox = getGameObjectBoundingBox(sObject.pObject, true); bbox.has_value() && m_camera.canSeeObject(bbox.value()))
					{
						// Need to render it
						collectRenderEntriesIntoRenderList(sObject.pObject.get(), entries, stats, bIgnoreVisibility, true);

						visitedObjects.insert(sObject.pObject.get());
					}
				}
			}
		}

		// Add debug stuff
		if (m_bRenderPortals || m_bRenderRoomBoundingBox)
		{
			for (const auto &sRoomDef : m_rooms)
			{
				if (sRoomDef.mExitsDebugModel && m_bRenderPortals)
				{
					for (const auto &sMesh : sRoomDef.mExitsDebugModel->meshes)
					{
						render::RenderEntry &exitPlaneRenderEntry = entries.emplace_back();

						// Render params
						exitPlaneRenderEntry.iPrimitiveId = 0;
						exitPlaneRenderEntry.iMeshIndex = 0;
						exitPlaneRenderEntry.iTrianglesNr = 0;
						exitPlaneRenderEntry.renderTopology = sMesh.renderTopology.value_or(render::RenderTopology::RT_TRIANGLES);

						// World params
						exitPlaneRenderEntry.vPosition = glm::vec3(.0f);
						exitPlaneRenderEntry.mWorldTransform = glm::mat4(1.f);
						exitPlaneRenderEntry.mLocalOriginalTransform = glm::mat3(1.f);
						exitPlaneRenderEntry.pMesh = const_cast<render::Mesh *>(&sMesh);

						// Material
						render::RenderEntry::Material &material = exitPlaneRenderEntry.material;
						constexpr float kOpacity = 0.1f;
						material.vDiffuseColor = sMesh.defaultColor.value_or(glm::vec4(1.f, 1.f, 0.f, kOpacity));
						material.renderState = gamelib::mat::MATRenderState("#BMEDIT/OPACITY_AREA",
						                                                    true, true, true, false, false,
						                                                    kOpacity,
						                                                    0.f,
						                                                    255,
						                                                    gamelib::mat::MATCullMode::CM_DontCare,
						                                                    gamelib::mat::MATBlendMode::BM_ADD,
						                                                    gamelib::mat::MATValU());
						material.pShader = &m_resources->m_shaders[m_resources->m_iGizmoShaderIdx];
					}
				}

				if (sRoomDef.mBBoxModel && m_bRenderRoomBoundingBox)
				{
					for (const auto &sMesh : sRoomDef.mBBoxModel->meshes)
					{
						render::RenderEntry &lineRenderEntry = entries.emplace_back();

						// Render params
						lineRenderEntry.iPrimitiveId = 0;
						lineRenderEntry.iMeshIndex = 0;
						lineRenderEntry.iTrianglesNr = 0;
						lineRenderEntry.renderTopology = sMesh.renderTopology.value_or(render::RenderTopology::RT_LINES);

						// World params
						lineRenderEntry.vPosition = glm::vec3(.0f);
						lineRenderEntry.mWorldTransform = glm::mat4(1.f);
						lineRenderEntry.mLocalOriginalTransform = glm::mat3(1.f);
						lineRenderEntry.pMesh = const_cast<render::Mesh *>(&sMesh);

						// Material
						render::RenderEntry::Material &material = lineRenderEntry.material;
						constexpr float kOpacity = 0.1f;
						material.vDiffuseColor = sMesh.defaultColor.value_or(glm::vec4(1.f, 0.f, 0.f, kOpacity));
						material.renderState = gamelib::mat::MATRenderState("#BMEDIT/OPACITY_AREA",
						                                                    true, true, true, false, false,
						                                                    kOpacity,
						                                                    0.f,
						                                                    255,
						                                                    gamelib::mat::MATCullMode::CM_DontCare,
						                                                    gamelib::mat::MATBlendMode::BM_ADD,
						                                                    gamelib::mat::MATValU());
						material.pShader = &m_resources->m_shaders[m_resources->m_iGizmoShaderIdx];
					}
				}
			}
		}

		// Post sorting
		entries.sort([&camera](const render::RenderEntry& a, const render::RenderEntry& b) -> bool {
			// Check distance to camera
			const float fADistanceToCamera = glm::length(camera.getPosition() - a.vPosition);
			const float fBDistanceToCamera = glm::length(camera.getPosition() - b.vPosition);

			return fADistanceToCamera > fBDistanceToCamera;
		});
	}

	void SceneRenderWidget::collectRenderEntriesIntoRenderList(const gamelib::scene::SceneObject* geom, render::RenderEntriesList& entries, RenderStats& stats, bool bIgnoreVisibility, bool bBreakOnChild) // NOLINT(*-no-recursion)
	{
		const bool bInvisible = geom->getProperties().getObject<bool>("Invisible", false);
		const auto vPosition  = geom->getPosition();
		auto primId = getGameObjectPrimitiveId(geom);

		// Calculate object world space bounding box and check that this bbox is visible by out camera

		if (const auto& n = geom->getType()->getName(); n == "ZSHADOWMESHOBJ" || n == "ZBOUND" || n == "ZLIGHT" || n == "ZENVIRONMENT" || n == "ZOMNILIGHT" || n == "ZSPOTLIGHT" || n == "ZSPOTLIGHTSQUARE")
		{
			// Do not draw us & our children
			return;
		}

		// Don't draw invisible things
		if (bInvisible)
			return;

		if (g_bannedObjectIds.contains(std::string_view{geom->getName()}) || geom->getName().starts_with("CloneGroup_"))
			return;

		// Check that our 'object' is not a collision box
		if (auto parent = geom->getParent().lock(); parent && parent->getType()->getName() == "ZROOM" && parent->getName() == geom->getName())
			return; // Do not render collision meshes

		// Check that object could be rendered by any way
		if (primId != 0 && m_resources->m_modelsCache.contains(primId))
		{
			glm::mat4 mWorldTransform = getGameObjectTransform(geom);

			// Get model
			const Model& model = m_resources->m_models[m_resources->m_modelsCache[primId]];
			gamelib::BoundingBox modelWorldBoundingBox = gamelib::BoundingBox::toWorld(model.boundingBox, mWorldTransform);

			if (m_camera.canSeeObject(glm::vec3(modelWorldBoundingBox.min), glm::vec3(modelWorldBoundingBox.max))) {
				// Add bounding box to render list
				{
					if (geom == m_pSelectedSceneObject && model.boundingBoxMesh.has_value()) {
						// Need to add mesh
						render::RenderEntry &boundingBoxEntry = entries.emplace_back();

						// Render params
						boundingBoxEntry.iPrimitiveId = 0;
						boundingBoxEntry.iMeshIndex = 0;
						boundingBoxEntry.iTrianglesNr = 0;
						boundingBoxEntry.renderTopology = render::RenderTopology::RT_LINES;

						// World params
						boundingBoxEntry.vPosition = vPosition;
						boundingBoxEntry.mWorldTransform = mWorldTransform;
						boundingBoxEntry.mLocalOriginalTransform = geom->getOriginalTransform();
						boundingBoxEntry.pMesh = const_cast<render::Mesh *>(&model.boundingBoxMesh.value());

						// Material
						render::RenderEntry::Material &material = boundingBoxEntry.material;
						material.vDiffuseColor = glm::vec4(0.f, 0.f, 1.f, 1.f);
						material.pShader = &m_resources->m_shaders[m_resources->m_iGizmoShaderIdx];
					}
				}

				// increase allowed objects count
				stats.allowedObjects++;

				// Add each 'mesh' into render list
				for (int iMeshIdx = 0; iMeshIdx < model.meshes.size(); iMeshIdx++) {
					const auto &mesh = model.meshes[iMeshIdx];

					if (mesh.materialId == 0)
						continue;// Unable to render (ZWINPIC!)

					// Filter by 'MeshVariantId'
					const auto requiredVariationId = geom->getProperties().getObject<std::int32_t>("MeshVariantId", 0);
					if (requiredVariationId != mesh.variationId) {
						continue;
					}

					// And store entry to renderer
					render::RenderEntry renderEntry = {};

					// Render params
					renderEntry.iPrimitiveId = primId;
					renderEntry.iMeshIndex = iMeshIdx;
					renderEntry.iTrianglesNr = mesh.trianglesCount;
					renderEntry.renderTopology = render::RenderTopology::RT_TRIANGLES;

					// World params
					renderEntry.vPosition = vPosition;
					renderEntry.mWorldTransform = mWorldTransform;
					renderEntry.mLocalOriginalTransform = geom->getOriginalTransform();
					renderEntry.pMesh = const_cast<render::Mesh *>(&mesh);

					// Material
					render::RenderEntry::Material &material = renderEntry.material;

					const auto &instances = m_pLevel->getLevelMaterials()->materialInstances;
					const auto &matInstance = instances[mesh.materialId - 1];

					// Store parameters
					material.id = mesh.materialId;
					material.sInstanceMatName = matInstance.getName();
					material.sBaseMatClass = matInstance.getParentName();

					if (!matInstance.getBinders().empty()) {
						const auto &binder = matInstance.getBinders()[0];// NOTE: In future I'll rewrite this place, but for now it's enough

						// Store parameters
						// TODO: Need collect all parameters here

						// Store render state
						if (!binder.renderStates.empty()) {
							// TODO: In future we need to learn how to use multiple render states (if there are able to be 'multiple')
							material.renderState = binder.renderStates[0];
						}

						if (!material.renderState.isEnabled())
						{
							// unable to see disabled material instance
							continue;
						}

						// Resolve & store textures
						std::fill(material.textures.begin(), material.textures.end(), kInvalidResource);

						for (const auto &texture : binder.textures) {
							if (texture.getPresentedTextureSources() == gamelib::mat::PresentedTextureSource::PTS_NOTHING)
								continue;// No texture at all

							if (texture.getPresentedTextureSources() == gamelib::mat::PresentedTextureSource::PTS_TEXTURE_ID_AND_PATH) {
								assert(false && "Idk how to handle this");
								continue;
							}

							const auto &kind = texture.getName();

							int textureSlotId = render::TextureSlotId::kMaxTextureSlot;

#define MATCH_TEXTURE_KIND(mode, modeName) if (kind == modeName) { textureSlotId = mode; }
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapDiffuse, "mapDiffuse")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapSpecularMask, "mapSpecularMask")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapEnvironment, "mapEnvironment")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapReflectionMask, "mapReflectionMask")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapReflectionFallOff, "mapReflectionFallOff")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapIllumination, "mapIllumination")
							MATCH_TEXTURE_KIND(render::TextureSlotId::kMapTranslucency, "mapTranslucency")
#undef MATCH_TEXTURE_KIND

							if (textureSlotId == render::TextureSlotId::kMaxTextureSlot)
								continue;

							// Now we need to find texture instance and associate it
							if (texture.getPresentedTextureSources() == gamelib::mat::PresentedTextureSource::PTS_TEXTURE_ID) {
								// Lookup in cache by texture id
								if (auto it = m_resources->m_textureIndexToGL.find(texture.getTextureId()); it != m_resources->m_textureIndexToGL.end()) {
									material.textures[textureSlotId] = it->second;
									break;
								}
							}

							if (texture.getPresentedTextureSources() == gamelib::mat::PresentedTextureSource::PTS_TEXTURE_PATH) {
								// Lookup in cache by texture path
								if (auto it = m_resources->m_textureNameToGL.find(texture.getTexturePath()); it != m_resources->m_textureNameToGL.end()) {
									material.textures[textureSlotId] = it->second;
									break;
								}
							}
						}
					}

					// Store shader
					material.pShader = &m_resources->m_shaders[m_resources->m_iTexturedShaderIdx];

					// Push or not?
					if (!std::all_of(material.textures.begin(), material.textures.end(), [](const auto &v) -> bool { return v == kInvalidResource; })) {
						entries.emplace_back(renderEntry);
					}
				}
			}
			else
			{
				// Increase rejected objects
				stats.rejectedObjects++;
			}
		}

		if (bBreakOnChild)
			return;

		// Visit others
		for (const auto& child : geom->getChildren())
		{
			if (auto g = child.lock())
			{
				collectRenderEntriesIntoRenderList(g.get(), entries, stats, bIgnoreVisibility);
			}
		}
	}

	void SceneRenderWidget::performRender(QOpenGLFunctions_3_3_Core* glFunctions, const render::RenderEntriesList& entries, const render::Camera& camera, const std::function<bool(const render::RenderEntry&)>& filter)
	{
		glm::ivec2 viewResolution = getViewportSize();

		auto applyRenderState = [](QOpenGLFunctions_3_3_Core* gapi, const gamelib::mat::MATRenderState& renderState)
		{
			// Enable or disable blending
			if (renderState.isBlendEnabled()) {
				gapi->glEnable(GL_BLEND);

				// Set blend mode based on your enum values
				switch (renderState.getBlendMode())
				{
				case gamelib::mat::MATBlendMode::BM_TRANS:
					gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case gamelib::mat::MATBlendMode::BM_TRANS_ON_OPAQUE:
					gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case gamelib::mat::MATBlendMode::BM_TRANSADD_ON_OPAQUE:
					gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
				case gamelib::mat::MATBlendMode::BM_ADD_BEFORE_TRANS:
					gapi->glBlendFunc(GL_ONE, GL_ONE);
					break;
				case gamelib::mat::MATBlendMode::BM_ADD_ON_OPAQUE:
					gapi->glBlendFunc(GL_ONE, GL_ONE);
					break;
				case gamelib::mat::MATBlendMode::BM_ADD:
					gapi->glBlendFunc(GL_ONE, GL_ONE);
					gapi->glEnable(GL_BLEND);
					break;
				default:
					// Do nothing
					break;
				}
			} else {
				gapi->glDisable(GL_BLEND);
			}

			// Enable or disable alpha testing
			if (renderState.isAlphaTestEnabled()) {
				gapi->glEnable(GL_ALPHA_TEST);
			} else {
				gapi->glDisable(GL_ALPHA_TEST);
			}

			// Enable or disable fog
			if (renderState.isFogEnabled()) {
				gapi->glEnable(GL_FOG);
			} else {
				gapi->glDisable(GL_FOG);
			}

#if 0
			// Enable or disable depth offset (Z bias)
			if (renderState.hasZBias()) {
				gapi->glEnable(GL_POLYGON_OFFSET_FILL);
				gapi->glPolygonOffset(2.0f, renderState.getZOffset());
			} else {
				gapi->glDisable(GL_POLYGON_OFFSET_FILL);
			}
#endif

			// Set cull mode based on your enum values
			switch (renderState.getCullMode())
			{
			case gamelib::mat::MATCullMode::CM_OneSided:
				gapi->glCullFace(GL_BACK);
				break;
			case gamelib::mat::MATCullMode::CM_DontCare:
			case gamelib::mat::MATCullMode::CM_TwoSided:
				// please complete
				gapi->glDisable(GL_CULL_FACE);
				break;
			}
		};

		static constexpr std::array<std::string_view, render::TextureSlotId::kMaxTextureSlot> g_aTextureKindToLocation {
		    "i_uMaterial.mapDiffuse",
		    "i_uMaterial.mapSpecularMask",
		    "i_uMaterial.mapEnvironment",
		    "i_uMaterial.mapReflectionMask",
		    "i_uMaterial.mapReflectionFallOff",
		    "i_uMaterial.mapIllumination",
		    "i_uMaterial.mapTranslucency"
		};

		for (const auto& entry : entries)
		{
			if (!filter(entry))
				continue; // skipped by filter

			// Switch render state
			applyRenderState(glFunctions, entry.material.renderState);

			// Activate material & setup parameters
			render::Shader* shader = entry.material.pShader;
			shader->bind(glFunctions);

			// Setup parameters (common)
			shader->setUniform(glFunctions, ShaderConstants::kModelTransform, entry.mWorldTransform);
			shader->setUniform(glFunctions, ShaderConstants::kCameraProjection, m_camera.getProjection());
			shader->setUniform(glFunctions, ShaderConstants::kCameraView, m_camera.getView());
			shader->setUniform(glFunctions, ShaderConstants::kCameraResolution, viewResolution);

			// TODO: Need to move into constants
			shader->setUniform(glFunctions, "i_uMaterial.v4DiffuseColor", entry.material.vDiffuseColor);
			shader->setUniform(glFunctions, "i_uMaterial.gm_vZBiasOffset", entry.material.renderState.hasZBias() ? entry.material.gm_vZBiasOffset : glm::vec4(0.f));
			shader->setUniform(glFunctions, "i_uMaterial.v4Opacity", entry.material.v4Opacity);
			shader->setUniform(glFunctions, "i_uMaterial.v4Bias", entry.material.v4Bias);
			shader->setUniform(glFunctions, "i_uMaterial.alphaREF", std::clamp(entry.material.iAlphaREF, 0, 255));
			shader->setUniform(glFunctions, "i_uMaterial.fZOffset", entry.material.renderState.getZOffset());

			// Bind textures
			for (int slotIdx = render::TextureSlotId::kMapDiffuse; slotIdx < render::TextureSlotId::kMaxTextureSlot; slotIdx++)
			{
				const auto& glTexture = entry.material.textures[slotIdx];

				if (glTexture == kInvalidResource)
					continue;

				glFunctions->glActiveTexture(GL_TEXTURE0 + slotIdx);
				glFunctions->glBindTexture(GL_TEXTURE_2D, glTexture);
				shader->setUniform(glFunctions, std::string(g_aTextureKindToLocation[slotIdx]), slotIdx);
			}

			if (m_renderMode & RenderMode::RM_TEXTURE)
			{
				entry.pMesh->render(glFunctions, entry.renderTopology);
			}

			if (m_renderMode & RenderMode::RM_WIREFRAME)
			{
				glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				entry.pMesh->render(glFunctions, entry.renderTopology);
				glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			// Release stuff
			for (int slotIdx = render::TextureSlotId::kMapDiffuse; slotIdx < render::TextureSlotId::kMaxTextureSlot; slotIdx++)
			{
				glFunctions->glActiveTexture(GL_TEXTURE0 + slotIdx);
				glFunctions->glBindTexture(GL_TEXTURE_2D, 0);
			}

			// And it's done
			shader->unbind(glFunctions);
		}
	}

	void SceneRenderWidget::invalidateRenderList()
	{
		m_renderList.clear();
	}

	void SceneRenderWidget::computeRoomBoundingBox(RoomDef& d)
	{
		using R = gamelib::scene::SceneObject::EVisitResult;

		if (auto pRoom = d.rRoom.lock())
		{
			// First of all we need try to lookup for 'CollisionMesh'. It has same name to room and be an STDOBJ
			const auto& children = pRoom->getChildren();
			gamelib::scene::SceneObject* pCollisionMesh = nullptr;
			pRoom->visitChildren([&pCollisionMesh, sTargetName = pRoom->getName()](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (pObject->getName() == sTargetName/* && pObject->getType()->getName() == "ZSTDOBJ"*/)
				{
					pCollisionMesh = pObject.get();
					return R::VR_STOP_ALL;
				}

				return R::VR_NEXT; // Do not go deeper
			});

			if (pCollisionMesh)
			{
				auto iPrimId = getGameObjectPrimitiveId(pCollisionMesh);
				if (iPrimId != 0)
				{
					// Nice, collision mesh was found! Just use it as source for bbox of ZROOM
					auto sBoundingBox = m_resources->m_models[m_resources->m_modelsCache[iPrimId]].boundingBox;
					d.vBoundingBox = gamelib::BoundingBox::toWorld(sBoundingBox, pCollisionMesh->getWorldTransform());
					d.eBoundingBoxSource = RoomDef::EBoundingBoxSource::BBS_ROOM_COLLISION_MESH;
					return;
				}
			}

			// Ok, let's try to find all ZBOUND objects and make BoundingBox
			gamelib::BoundingBox sTempBbox {};
			int iZBoundObjsFound = 0;
			pRoom->visitChildren([this, &sTempBbox, &iZBoundObjsFound](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (pObject && pObject->getType()->getName() == "ZBOUND")
				{
					if (auto bbox = getGameObjectBoundingBox(pObject.get()); bbox.has_value())
					{
						++iZBoundObjsFound;
						sTempBbox.expand(bbox.value());
					}
				}

				return R::VR_NEXT; // Never go inside
			});

			if (iZBoundObjsFound > 0)
			{
				d.vBoundingBox = sTempBbox;
				d.eBoundingBoxSource = RoomDef::EBoundingBoxSource::BBS_ZBOUNDS_AUTO_EXPAND;
				return;
			}

			// Old and hard way: just collect all visible objects with bboxes and combine them all into 1 single big bbox
			bool bBboxInited = false;

			pRoom->visitChildren([&](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (!pObject)
					return R::VR_NEXT;

				if (auto bbox = getGameObjectBoundingBox(pObject.get()); bbox.has_value())
				{
					if (!bBboxInited)
					{
						bBboxInited = true;
						d.vBoundingBox = bbox.value();
					}
					else
					{
						d.vBoundingBox.expand(bbox.value());
					}

					return R::VR_NEXT;
				}

				return R::VR_CONTINUE;
			});

			d.eBoundingBoxSource = RoomDef::EBoundingBoxSource::BBS_AUTO_ROOM_EXPAND;
		}
	}

	void SceneRenderWidget::buildRoomCache(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		using R = gamelib::scene::SceneObject::EVisitResult;

		// clear caches
		m_rooms.clear();
		m_cameraInRooms.clear();

		// Save pointer to  BUF file
		const auto bufFileView = m_pLevel->getStaticBuffer();

		// Now we need to find ZGROUP who ends by _LOCATIONS and lookup from this ZGROUP inside
		auto locationsIt = std::find_if(
		    m_pLevel->getSceneObjects().begin(),
		    m_pLevel->getSceneObjects().end(),
		    [](const gamelib::scene::SceneObject::Ptr& pObject) -> bool {
			    return pObject && pObject->getName().ends_with("_LOCATIONS.zip");
		    });

		if (locationsIt != m_pLevel->getSceneObjects().end())
		{
			// we've able to use standard workflow
			// Find ZROOMs
			const gamelib::scene::SceneObject::Ptr& pNewRoot = *locationsIt;

			pNewRoot->visitChildren([this, bufFileView](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (!pObject)
				{
					return R::VR_STOP_ALL;
				}

				if (pObject->getType()->getName() == "ZROOM")
				{
					// Add and go next, do not go inside
					auto& room = m_rooms.emplace_back();
					room.rRoom = pObject;

					//room.eLocation
					static const std::map<std::string, RoomDef::ELocation> s_LocNameToKind {
					    { "eBOTH", RoomDef::ELocation::eBOTH },
					    { "eINSIDE", RoomDef::ELocation::eINSIDE },
					    { "eOUTSIDE", RoomDef::ELocation::eOUTSIDE },
					    { "eUNDEFINED", RoomDef::ELocation::eUNDEFINED }
					};
					const auto sLocation = pObject->getProperties().getObject<std::string>("Location", "");

					if (auto it = s_LocNameToKind.find(sLocation); it != s_LocNameToKind.end())
					{
						room.eLocation = it->second;
					}
					else
					{
						room.eLocation = RoomDef::ELocation::eUNDEFINED;
						assert(false && "Unknown room type, room will be ignored in optimisations loop");
					}

					//room.iExitsCount, room.ExitOffsets (Precache room exit boxes)
					const auto iExistsCount = pObject->getProperties().getObject<std::int32_t>("iExitsCount", 0);
					const auto iExistsOffset = pObject->getProperties().getObject<std::int32_t>("ExitOffsets", 0);
					if (iExistsCount > 0 && iExistsOffset > 0)
					{
						room.aExists.reserve(iExistsCount);

						// Take a slice of data
						constexpr auto kEntrySize = static_cast<int64_t>(sizeof(gamelib::gms::room::ZRoomExit));
						const auto roomExists = bufFileView.slice(iExistsOffset, kEntrySize * iExistsCount);

						for (int i = 0; i < iExistsCount; i++)
						{
							const auto exit = roomExists.slice((i * kEntrySize), kEntrySize);
							auto& exitDef = room.aExists.emplace_back();
							gamelib::gms::room::ZRoomExit::deserialize(exitDef, exit);
						}
					}

					//room.iNeighboursCount, room.NeighborsOffset
					const auto iNeighboursCount = pObject->getProperties().getObject<std::int32_t>("iNeighboursCount", 0);
					const auto iNeighborsOffset = pObject->getProperties().getObject<std::int32_t>("NeighborsOffset", 0);
					if (iNeighboursCount > 0 && iNeighborsOffset > 0)
					{
						room.aNeighbours.reserve(iNeighboursCount);

						constexpr auto kEntrySize = static_cast<int64_t>(sizeof(gamelib::gms::room::ZRoomNeighbor));
						const auto roomNeighbours = bufFileView.slice(iNeighborsOffset, kEntrySize * iNeighboursCount);

						for (int i = 0; i < iNeighboursCount; i++)
						{
							const auto neighbour = roomNeighbours.slice((i * kEntrySize), kEntrySize);
							auto& neighbourDef = room.aNeighbours.emplace_back();
							gamelib::gms::room::ZRoomNeighbor::deserialize(neighbourDef, neighbour);
						}
					}

					// Compute room dimensions
					computeRoomBoundingBox(room);

					// Collect objects list (all visible objects from room + dynamics from scene)
					pObject->visitChildren([&room, this](const gamelib::scene::SceneObject::Ptr& pObj) -> R {
						if (pObj->is("ZROOM")) return R::VR_CONTINUE;

						if (auto bbox = getGameObjectBoundingBox(pObj); bbox.has_value())
						{
							SeebleObject& sObject = room.vObjects.emplace_back();
							sObject.pObject = pObj;
							sObject.ePrio = EObjectPriority::EP_STATIC_OBJECT;
							return R::VR_NEXT; // Go to next
						}
						return R::VR_CONTINUE; // go deeper
					});

					return R::VR_NEXT;
				}

				// Go deep inside
				return R::VR_CONTINUE;
			});
		}
		else
		{
			// No rooms found. Need to generate 1 big room
			RoomDef& sVirtualRoom = m_rooms.emplace_back();

			// Use really huge bbox (FLT32_MIN;FLT32_MIN;FLT32_MIN) (FLT32_MAX; FLT32_MAX; FLT32_MAX)
			sVirtualRoom.vBoundingBox = gamelib::BoundingBox(
			    glm::vec3(
			        std::numeric_limits<float>::min(),
			        std::numeric_limits<float>::min(),
			        std::numeric_limits<float>::min()
				),
			    glm::vec3(
			        std::numeric_limits<float>::max(),
			        std::numeric_limits<float>::max(),
			        std::numeric_limits<float>::max()
				)
			);

			// Set location & flags
			sVirtualRoom.eLocation = RoomDef::ELocation::eUNDEFINED;
			sVirtualRoom.bIsVirtualBigRoom = true;

			// Collect objects
			m_pLevel->getSceneObjects()[0]->visitChildren([&sVirtualRoom, this](const gamelib::scene::SceneObject::Ptr& pObj) -> R {
				if (pObj->is("ZROOM")) return R::VR_CONTINUE;

				if (auto bbox = getGameObjectBoundingBox(pObj); bbox.has_value())
				{
					SeebleObject& sObject = sVirtualRoom.vObjects.emplace_back();
					sObject.pObject = pObj;
					sObject.ePrio = EObjectPriority::EP_STATIC_OBJECT; // Idk, but in this case all objects are STATIC
					return R::VR_NEXT; // Go to next
				}

				return R::VR_CONTINUE; // go deeper
			});
		}

		if (m_rooms.size() > 1 && !m_rooms.begin()->bIsVirtualBigRoom)
		{
			// Visit all objects before any rooms
			m_pLevel->getSceneObjects()[0]->visitChildren([this](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (pObject->getName() == "Scar!scar")
				{
					printf("DEBUG\n");
				}
				if (pObject->isInheritedOf("ZROOM")) return R::VR_NEXT; // Skip current branch

				if (auto rBBOX = getGameObjectBoundingBox(pObject, true); rBBOX.has_value())
				{
					// Need to find in which room this subject should be
					for (auto& sRoom : m_rooms)
					{
						if (sRoom.vBoundingBox.intersect(rBBOX.value()))
						{
							// Nice, save here
							SeebleObject& sObject = sRoom.vObjects.emplace_back();
							sObject.pObject = pObject;
							sObject.ePrio = EObjectPriority::EP_DYNAMIC_OBJECT; // mark as dynamic

							// break; // DronCode: Need to fix global bboxes before work with it.
						}
					}

					// Skip subtree (no visible inside)
					return R::VR_NEXT;
				} // skipped, no real reason to handle invisible object (but object could be visible after prop changed. be aware)

				return R::VR_CONTINUE; // go deeper
			});
		}

		// Upload debug geom
		for (auto& room : m_rooms)
		{
			if (!room.aExists.empty())
			{
				room.mExitsDebugModel = std::make_unique<render::Model>();

				for (const auto& sExit : room.aExists)
				{
					gamelib::Plane sPlane { sExit.v0, sExit.v1, sExit.v2, sExit.v3 };

					// Plane
					{
						auto& exitMesh = room.mExitsDebugModel->meshes.emplace_back();
						exitMesh.glTextureId = render::kInvalidResource;
						exitMesh.materialId = 0;
						exitMesh.renderTopology = RenderTopology::RT_TRIANGLES;

						std::vector<render::SimpleVertex> aVertices;
						std::vector<uint16_t> aIndices;

						sPlane.toTriangles(std::back_inserter(aVertices), std::back_inserter(aIndices));

						exitMesh.setup(glFunctions, render::SimpleVertex::g_FormatDescription, aVertices, aIndices, false);
					}

					// Normal vector direction
					{
						auto& exitMeshNormalView = room.mExitsDebugModel->meshes.emplace_back();
						exitMeshNormalView.glTextureId = render::kInvalidResource;
						exitMeshNormalView.materialId = 0;
						exitMeshNormalView.renderTopology = RenderTopology::RT_LINES;
						exitMeshNormalView.defaultColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

						const float kVecLength = sPlane.getSize() * 0.2f; //20% of size

						std::vector<render::SimpleVertex> aVertices {
						    sPlane.getCenter(),
						    sPlane.getCenter() + (sPlane.getNormal() * kVecLength),
						    sPlane.getCenter() - (sPlane.getNormal() * kVecLength)
						};

						std::vector<uint16_t> aIndices { 0, 1, 0, 2 };

						exitMeshNormalView.setup(glFunctions, render::SimpleVertex::g_FormatDescription, aVertices, aIndices, false);
					}
				}
			}

			// Add bounding box model
			{
				room.mBBoxModel = std::make_unique<render::Model>();

				auto& bboxMesh = room.mBBoxModel->meshes.emplace_back();
				bboxMesh.glTextureId = render::kInvalidResource;
				bboxMesh.materialId = 0;
				bboxMesh.renderTopology = RenderTopology::RT_LINES;

				std::vector<render::SimpleVertex> aVertices;
				std::vector<uint16_t> aIndices;

				room.vBoundingBox.toLines(std::back_inserter(aVertices), std::back_inserter(aIndices));
				bboxMesh.setup(glFunctions, render::SimpleVertex::g_FormatDescription, aVertices, aIndices, false);
			}
		}
	}

	void SceneRenderWidget::updateCameraRoomAttachment(RenderStats& stats, bool bRejectLastResult)
	{
		m_cameraInRooms.clear();

		for (const auto& sRoom : m_rooms)
		{
			if (sRoom.vBoundingBox.contains(m_camera.getPosition()))
			{
				m_cameraInRooms.emplace_back(&sRoom);
			}
		}

#if 0
		/**
		 * Here is a place from hell. We need to know in which room camera and what rooms we can see from this place.
		 *
		 * First:
		 * 		IDK how to solve
		 *
		 * Second:
		 * 		Each room has "exits" and "neighbours". We just need to  check what planes we can see from this room and this pos + dir (camera)
		 */
		std::list<const RoomDef*> roomCandidates {};

		for (const auto& sRoom : m_rooms)
		{
			if (sRoom.vBoundingBox.contains(m_camera.getPosition()))
			{
				roomCandidates.emplace_back(&sRoom);
			}
		}

		if (roomCandidates.empty())
			return;

		roomCandidates.sort([](const RoomDef* a, const RoomDef* b) { return a->eLocation < b->eLocation; });

		RoomDef::ELocation currentLocation = (*roomCandidates.begin())->eLocation;

		for (const auto& sRoom : roomCandidates)
		{
			if (sRoom->eLocation != currentLocation)
				continue;

			m_cameraInRooms.emplace_back(sRoom);
		}
#endif
	}
}