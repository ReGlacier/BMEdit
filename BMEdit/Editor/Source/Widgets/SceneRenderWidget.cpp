#include <Widgets/SceneRenderWidget.h>
#include <Editor/TextureProcessor.h>
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
#include <GameLib/PRP/PRPMathTypes.h>
#include <GameLib/BoundingBox.h>

#include <Render/ShaderConstants.h>
#include <Render/GlacierVertex.h>
#include <Render/GLResource.h>
#include <Render/Texture.h>
#include <Render/Shader.h>
#include <Render/Model.h>

#include <unordered_map>
#include <algorithm>
#include <optional>


namespace widgets
{
	using namespace render;

	struct SceneRenderWidget::GLResources
	{
		std::vector<Texture> m_textures {};
		std::vector<Shader> m_shaders {};
		std::vector<Model> m_models {};
		std::unordered_map<uint32_t, size_t> m_modelsCache {};  /// primitive index to model index in m_models
		std::unordered_map<gamelib::scene::SceneObject*, glm::mat4> m_modelTransformCache {}; /// transformations cache
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
		auto funcs = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
		if (!funcs) {
			qFatal("Could not obtain required OpenGL context version");
			return;
		}

		// Begin frame
		funcs->glViewport(0, 0, QWidget::width(), QWidget::height());
		funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		funcs->glClearColor(0.15f, 0.2f, 0.45f, 1.0f);

		// Z-Buffer testing
		funcs->glEnable(GL_DEPTH_TEST);

		// Blending
		funcs->glEnable(GL_BLEND);
		funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (m_bDirtyProj)
		{
			updateProjectionMatrix(QWidget::width(), QWidget::height());
		}

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
				    doCollectRenderList(m_camera, pRoot, m_renderList, bIgnoreVisibility);
			    }

			    if (!m_renderList.empty())
			    {
				    doPerformDrawOfRenderList(funcs, m_renderList, m_camera);
			    }
		    }
		    break;
		}
	}

	void SceneRenderWidget::resizeGL(int w, int h)
	{
		Q_UNUSED(w);
		Q_UNUSED(h);

		// Update projection
		updateProjectionMatrix(w, h);

		// Because our list of visible objects could be changed here
		invalidateRenderList();
	}

	void SceneRenderWidget::keyPressEvent(QKeyEvent* event)
	{
		if (m_pLevel)
		{
			bool bMoved = false;
			constexpr float kBaseDt = 1.f / 60.f;
			float kSpeedUp = 100.f;

			if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
				    kSpeedUp *= 4.f;


			if (event->key() == Qt::Key_W)
			{
				m_camera.processKeyboard(render::Camera_Movement::FORWARD, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_S)
			{
				m_camera.processKeyboard(render::Camera_Movement::BACKWARD, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_A)
			{
				m_camera.processKeyboard(render::Camera_Movement::LEFT, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_D)
			{
				m_camera.processKeyboard(render::Camera_Movement::RIGHT, kBaseDt, kSpeedUp);
				bMoved = true;
			}

			if (bMoved)
			{
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
				m_camera.processMouseMovement(xOffset, yOffset);
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

	void SceneRenderWidget::updateProjectionMatrix(int w, int h)
	{
		m_matProjection = glm::perspectiveFovLH(glm::radians(m_fFOV), static_cast<float>(w), static_cast<float>(h), m_fZNear, m_fZFar);
		m_bDirtyProj = false;
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
			Texture newTexture;

			std::unique_ptr<std::uint8_t[]> decompressedMemBlk = editor::TextureProcessor::decompressRGBA(texture, newTexture.width, newTexture.height, 0); //
			if (!decompressedMemBlk)
			{
				m_resources->m_textures.emplace_back();
				qWarning() << "Failed to decompress texture #" << texture.m_index << " to RGBA sequence";
				continue;
			}

			// Store texture index from TEX container
			newTexture.index = std::make_optional(texture.m_index);
			newTexture.texPath = texture.m_fileName;  // just copy file name from tex (if it defined!)

			// Create GL resource
			glFunctions->glGenTextures(1, &newTexture.texture);
			glFunctions->glBindTexture(GL_TEXTURE_2D, newTexture.texture);
			glFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTexture.width, newTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, decompressedMemBlk.get());

			glFunctions->glGenerateMipmap(GL_TEXTURE_2D);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFunctions->glBindTexture(GL_TEXTURE_2D, 0);

			// Precache debug texture if it's not precached yet
			static constexpr const char* kGlacierMissingTex = "_Glacier/Missing_01";
			static constexpr const char* kWorldColiTex = "_TEST/Worldcoli";

			if (m_resources->m_iGLDebugTexture == 0 && texture.m_fileName.has_value() && (texture.m_fileName.value() == kGlacierMissingTex || texture.m_fileName.value() == kWorldColiTex))
			{
				m_resources->m_iGLDebugTexture = newTexture.texture;
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
			const auto iPrimId = player->getProperties().getObject<std::int32_t>("PrimId", 0u);
			const auto vPlayerPosition = player->getParent().lock()->getProperties().getObject<glm::vec3>("Position", glm::vec3(0.f));
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

	void SceneRenderWidget::doCollectRenderList(const render::Camera& camera, const gamelib::scene::SceneObject* pRootGeom, RenderList& renderList, bool bIgnoreVisibility)
	{
		doVisitGeomToCollectIntoRenderList(pRootGeom, renderList, bIgnoreVisibility);

		renderList.sort([&camera](const RenderEntry& a, const RenderEntry& b) -> bool {
			const float fADistanceToCamera = glm::length(camera.Position - a.vPosition);
			const float fBDistanceToCamera = glm::length(camera.Position - b.vPosition);

			return fADistanceToCamera > fBDistanceToCamera;
		});
	}

	void SceneRenderWidget::doVisitGeomToCollectIntoRenderList(const gamelib::scene::SceneObject* geom, RenderList& renderList, bool bIgnoreVisibility) // NOLINT(*-no-recursion)
	{
		const auto primId     = geom->getProperties().getObject<std::int32_t>("PrimId", 0);
		const bool bInvisible = geom->getProperties().getObject<bool>("Invisible", false);
		const auto vPosition  = geom->getProperties().getObject<glm::vec3>("Position", glm::vec3(0.f));

		// Don't draw invisible things
		if (bInvisible && !bIgnoreVisibility)
			return;

		// TODO: Check for culling here (object visible or not)

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

			// Store into render list
			RenderEntry& entry = renderList.emplace_back();
			entry.vPosition = vPosition;
			entry.mModelMatrix = mWorldTransform;
			entry.pGeom = geom;
			entry.iPrimId = primId;

			// Store bounding box
			const Model& model = m_resources->m_models[m_resources->m_modelsCache[entry.iPrimId]];
			entry.sBoundingBox.min = model.boundingBox.min;
			entry.sBoundingBox.max = model.boundingBox.max;
		}

		// Visit others
		for (const auto& child : geom->getChildren())
		{
			if (auto g = child.lock())
			{
				doVisitGeomToCollectIntoRenderList(g.get(), renderList, bIgnoreVisibility);
			}
		}
	}

	void SceneRenderWidget::doPerformDrawOfRenderList(QOpenGLFunctions_3_3_Core* glFunctions, const RenderList& renderList, const render::Camera& camera)
	{
		glm::ivec2 viewResolution { QWidget::width(), QWidget::height() };

		for (const auto& entry : renderList)
		{
			const Model& model = m_resources->m_models[m_resources->m_modelsCache[entry.iPrimId]];

			// Render bounding box
			if (model.boundingBoxMesh.has_value() && entry.pGeom == m_pSelectedSceneObject)
			{
				Shader& gizmoShader = m_resources->m_shaders[m_resources->m_iGizmoShaderIdx];
				gizmoShader.bind(glFunctions);

				gizmoShader.setUniform(glFunctions, ShaderConstants::kModelTransform, entry.mModelMatrix);
				gizmoShader.setUniform(glFunctions, ShaderConstants::kCameraProjection, m_matProjection);
				gizmoShader.setUniform(glFunctions, ShaderConstants::kCameraView, camera.getViewMatrix());
				gizmoShader.setUniform(glFunctions, ShaderConstants::kCameraResolution, viewResolution);
				gizmoShader.setUniform(glFunctions, ShaderConstants::kColor, glm::vec4(0.f, 0.f, 1.f, 1.f));

				model.boundingBoxMesh.value().render(glFunctions, RenderTopology::RT_LINES);

				gizmoShader.unbind(glFunctions);
			}

			// Render all meshes
			if (m_resources->m_modelsCache.contains(entry.iPrimId))
			{
				for (const auto& mesh : model.meshes)
				{
					// Render single mesh
					// 0. Check that we've able to draw it
					if (mesh.glTextureId == kInvalidResource)
					{
						// Draw "error" bounding box
						// And continue
						continue;
					}

					// Filter mesh by 'variation id'
					if (const auto& properties = entry.pGeom->getProperties(); properties.hasProperty("MeshVariantId"))
					{
						const auto requiredVariationId = properties.getObject<std::int32_t>("MeshVariantId", 0);
						if (requiredVariationId != mesh.variationId)
						{
							continue;
						}
					}

					// 1. Activate default shader
					Shader& texturedShader = m_resources->m_shaders[m_resources->m_iTexturedShaderIdx];

					texturedShader.bind(glFunctions);

					// 2. Submit uniforms
					texturedShader.setUniform(glFunctions, ShaderConstants::kModelTransform, entry.mModelMatrix);
					texturedShader.setUniform(glFunctions, ShaderConstants::kCameraProjection, m_matProjection);
					texturedShader.setUniform(glFunctions, ShaderConstants::kCameraView, m_camera.getViewMatrix());
					texturedShader.setUniform(glFunctions, ShaderConstants::kCameraResolution, viewResolution);

					// 3. Bind texture
					if (mesh.glTextureId != kInvalidResource)
					{
						glFunctions->glBindTexture(GL_TEXTURE_2D, mesh.glTextureId);
					}

					// 4. Render mesh
					if (m_renderMode & RenderMode::RM_TEXTURE)
					{
						// normal draw
						mesh.render(glFunctions);
					}

					if (m_renderMode & RenderMode::RM_WIREFRAME)
					{
						glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						mesh.render(glFunctions);
						// reset back
						glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}

					// 5. Unbind texture and shader (expected to switch between materials, but not now)
					glFunctions->glBindTexture(GL_TEXTURE_2D, 0);
					texturedShader.unbind(glFunctions);
				}
			}
		}
	}

	void SceneRenderWidget::invalidateRenderList()
	{
		m_renderList.clear();
	}
}