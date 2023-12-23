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

	void SceneRenderWidget::onRedrawRequested()
	{
		if (m_pLevel)
			repaint();
	}

	void SceneRenderWidget::onObjectMoved(gamelib::scene::SceneObject* sceneObject)
	{
		if (!sceneObject || !m_pLevel || !m_resources)
			return;

		m_resources->m_modelTransformCache[sceneObject] = sceneObject->getWorldTransform();
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
		buildRoomCache();

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
			const auto iPrimId = player->getProperties().getObject<std::int32_t>("PrimId", 0);
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
		if (m_pLevel->getSceneObjects().empty())
			return;

		if (pRootGeom != m_pLevel->getSceneObjects()[0].get() || m_rooms.empty() /* on some levels m_rooms cache could be not presented! */)
		{
			// Render from specific node
			collectRenderEntriesIntoRenderList(pRootGeom, entries, stats, bIgnoreVisibility);
		}
		else
		{
			std::list<RoomDef> acceptedRooms {};

			// Render static
			for (const auto& sRoomDef : m_rooms)
			{
				const bool bCameraInsideRoom = sRoomDef.vBoundingBox.contains(m_camera.getPosition());

				if (bCameraInsideRoom || m_camera.canSeeObject(sRoomDef.vBoundingBox.min, sRoomDef.vBoundingBox.max))
				{
					if (auto pRoom = sRoomDef.rRoom.lock())
					{
						if (bCameraInsideRoom)
						{
							// Store new room name
							stats.currentRoom = QString::fromStdString(pRoom->getName());
						}

						collectRenderEntriesIntoRenderList(pRoom.get(), entries, stats, bIgnoreVisibility);
					}

					acceptedRooms.emplace_back(sRoomDef);
				}
			}

			// Render dynamic
			const auto& vObjects = m_pLevel->getSceneObjects();
			auto it = std::find_if(vObjects.begin(), vObjects.end(), [](const gamelib::scene::SceneObject::Ptr& pObject) -> bool {
				return pObject && pObject->getName().ends_with("_CHARACTERS.zip");
			});

			if (it != vObjects.end())
			{
				// Add this 'ROOM' as another thing to visit
				const gamelib::scene::SceneObject* pDynRoot = it->get();

				using R = gamelib::scene::SceneObject::EVisitResult;
				pDynRoot->visitChildren([&entries, &stats, this](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
					if (pObject->getName().ends_with("_LOCATIONS.zip"))
						return R::VR_NEXT;

					// Collect everything inside
					collectRenderEntriesIntoRenderList(pObject.get(), entries, stats, false);
					return R::VR_NEXT;
				});
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

	void SceneRenderWidget::collectRenderEntriesIntoRenderList(const gamelib::scene::SceneObject* geom, render::RenderEntriesList& entries, RenderStats& stats, bool bIgnoreVisibility) // NOLINT(*-no-recursion)
	{
		const bool bInvisible = geom->getProperties().getObject<bool>("Invisible", false);
		const auto vPosition  = geom->getPosition();
		auto primId = geom->getProperties().getObject<std::int32_t>("PrimId", 0);

		// Calculate object world space bounding box and check that this bbox is visible by out camera

		if (const auto& n = geom->getType()->getName(); n == "ZSHADOWMESHOBJ" || n == "ZBOUND" || n == "ZLIGHT" || n == "ZENVIRONMENT" || n == "ZOMNILIGHT" || n == "ZSPOTLIGHT" || n == "ZSPOTLIGHTSQUARE")
		{
			// Do not draw us & our children
			return;
		}

		// Don't draw invisible things
		if (bInvisible && !bIgnoreVisibility)
			return;

		if (g_bannedObjectIds.contains(std::string_view{geom->getName()}))
			return;

		// Check that our 'object' is not a collision box
		if (auto parent = geom->getParent().lock(); parent && parent->getType()->getName() == "ZROOM" && parent->getName() == geom->getName())
			return; // Do not render collision meshes
		
		// NOTE: Need to refactor this place and move it into separated area
		// TODO: Move this hack into another place!
		if (geom->isInheritedOf("ZItem"))
		{
			//ZItems has no PrimId. Instead of this they are refs to another geom by path
			auto rItemTemplatePath = geom->getProperties().getObject<std::string>("rItemTemplate");
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
					primId = pItem->getProperties().getObject<std::int32_t>("PrimId");
				}
			}
		}

		// Check that object could be rendered by any way
		if (primId != 0 && m_resources->m_modelsCache.contains(primId))
		{
			glm::mat4 mWorldTransform = glm::mat4(1.f);

			if (auto it = m_resources->m_modelTransformCache.find(const_cast<gamelib::scene::SceneObject*>(geom)); it != m_resources->m_modelTransformCache.end())
			{
				mWorldTransform = it->second;
			}
			else
			{
				mWorldTransform = geom->getWorldTransform();
				m_resources->m_modelTransformCache[const_cast<gamelib::scene::SceneObject*>(geom)] = mWorldTransform;
			}

			// Get model
			const Model& model = m_resources->m_models[m_resources->m_modelsCache[primId]];
			gamelib::BoundingBox modelWorldBoundingBox = gamelib::BoundingBox::toWorld(model.boundingBox, mWorldTransform);

			if (m_camera.canSeeObject(glm::vec3(modelWorldBoundingBox.min), glm::vec3(modelWorldBoundingBox.max))) {
				// Add bounding box to render list
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
			} else {
				gapi->glDisable(GL_BLEND);
			}

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
				break;
			default:
				// Do nothing
				break;
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

			// Enable or disable depth offset (Z bias)
			if (renderState.hasZBias()) {
				gapi->glEnable(GL_POLYGON_OFFSET_FILL);
				gapi->glPolygonOffset(1.0f, renderState.getZOffset());
			} else {
				gapi->glDisable(GL_POLYGON_OFFSET_FILL);
			}

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
			shader->setUniform(glFunctions, "i_uMaterial.gm_vZBiasOffset", entry.material.gm_vZBiasOffset);
			shader->setUniform(glFunctions, "i_uMaterial.v4Opacity", entry.material.v4Opacity);
			shader->setUniform(glFunctions, "i_uMaterial.v4Bias", entry.material.v4Bias);
			shader->setUniform(glFunctions, "i_uMaterial.alphaREF", std::clamp(entry.material.iAlphaREF, 0, 255));

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
				if (pObject->getName() == sTargetName && pObject->getType()->getName() == "ZSTDOBJ")
				{
					pCollisionMesh = pObject.get();
					return R::VR_STOP_ALL;
				}

				return R::VR_NEXT; // Do not go deeper
			});

			if (pCollisionMesh)
			{
				auto iPrimId = pCollisionMesh->getProperties().getObject<std::int32_t>("PrimId", 0);
				if (iPrimId != 0)
				{
					// Nice, collision mesh was found! Just use it as source for bbox of ZROOM
					auto sBoundingBox = m_resources->m_models[m_resources->m_modelsCache[iPrimId]].boundingBox;
					d.vBoundingBox = gamelib::BoundingBox::toWorld(sBoundingBox, pCollisionMesh->getWorldTransform());
					return;
				}
			}

			bool bBboxInited = false;

			pRoom->visitChildren([&](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (!pObject)
					return R::VR_NEXT;

				auto iPrimId = pObject->getProperties().getObject<std::int32_t>("PrimId", 0);
				if (!iPrimId)
					return R::VR_CONTINUE; // Go deeper

				// Find prim cache
				if (m_resources->m_modelsCache.contains(iPrimId))
				{
					auto sBoundingBox = m_resources->m_models[m_resources->m_modelsCache[iPrimId]].boundingBox;
					gamelib::BoundingBox vWorldBoundingBox = gamelib::BoundingBox::toWorld(sBoundingBox, pObject->getWorldTransform());

					if (!bBboxInited)
					{
						bBboxInited = true;
						d.vBoundingBox = vWorldBoundingBox;
					}
					else
					{
						d.vBoundingBox.expand(vWorldBoundingBox);
					}

					// Optimisation: we've assumed that when we have an object with bbox we will use top bbox instead of compute sub-bboxes
					return R::VR_NEXT;
				}

				return R::VR_CONTINUE;
			});
		}
	}

	void SceneRenderWidget::buildRoomCache()
	{
		m_rooms.clear();

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
			// Save backdrop
			m_pLevel->forEachObjectOfType("ZBackdrop", [this](const gamelib::scene::SceneObject::Ptr& pObject) -> bool {
				if (pObject)
				{
					auto& room = m_rooms.emplace_back();
					room.rRoom = pObject;
					// ZBackdrop always has maximum possible size to see it from any point of the world
					constexpr float kMinPoint = std::numeric_limits<float>::min();
					constexpr float kMaxPoint = std::numeric_limits<float>::max();
					room.vBoundingBox = gamelib::BoundingBox(glm::vec3(kMinPoint), glm::vec3(kMaxPoint));

					return false;
				}

				return true;
			});

			// Find ZROOMs
			const gamelib::scene::SceneObject::Ptr& pNewRoot = *locationsIt;

			using R = gamelib::scene::SceneObject::EVisitResult;
			pNewRoot->visitChildren([this](const gamelib::scene::SceneObject::Ptr& pObject) -> R {
				if (!pObject)
				{
					return R::VR_STOP_ALL;
				}

				if (pObject->getType()->getName() == "ZROOM")
				{
					// Add and go next, do not go inside
					auto& room = m_rooms.emplace_back();
					room.rRoom = pObject;

					// Compute room dimensions
					computeRoomBoundingBox(room);

					return R::VR_NEXT;
				}

				// Go deep inside
				return R::VR_CONTINUE;
			});
		}
	}
}