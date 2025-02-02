#include <Widgets/SceneRenderWidget.h>
#include <Editor/TextureProcessor.h>

#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QSharedPointer>
#include <QOpenGLContext>
#include <QMessageBox>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QFile>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/PRP/PRPObjectExtractor.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <GameLib/BoundingBox.h>
#include <GameLib/Plane.h>

#include <Render/ShaderConstants.h>
#include <Render/GlacierVertex.h>
#include <Render/GLResource.h>
#include <Render/Shader.h>

#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <algorithm>
#include <chrono>
#include <set>


#ifdef BMEDIT_DEBUG
void BMEdit_OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	Q_UNUSED(length);
	Q_UNUSED(userParam);

	QString sourceStr;
	switch (source) {
		case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
	}

	QString typeStr;
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         return; //typeStr = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
	}

	QString severityStr;
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: return; // No need to see this spam later
	}

	qDebug() << "[DEBUG_BUILD] OpenGL Message:"
	         << "\nSource:" << sourceStr
	         << "\nType:" << typeStr
	         << "\nSeverity:" << severityStr
	         << "\nMessage:" << message
	         << "\nID:" << id;
}
#endif


namespace widgets
{
	int32_t GetSceneObjectPrimitiveID(const gamelib::Level* pLevel, const gamelib::scene::SceneObject* pObject);

	struct IndirectRenderDrawCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	struct GLExtFunctions
	{
		using Ptr = std::unique_ptr<GLExtFunctions>;

		explicit GLExtFunctions(QOpenGLContext* pContext);
		[[nodiscard]] bool IsAllFunctionsArePresentedAndSupported() const;

		typedef GLuint64 (*PFNGLGETTEXTUREHANDLEARBPROC)(GLuint texture);
		typedef void     (*PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)(GLuint64 handle);
		typedef void     (*PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC)(GLuint64 handle);

		PFNGLGETTEXTUREHANDLEARBPROC             glGetTextureHandleARB             = nullptr;
		PFNGLMAKETEXTUREHANDLERESIDENTARBPROC    glMakeTextureHandleResidentARB    = nullptr;
		PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC glMakeTextureHandleNonResidentARB = nullptr;
	};

	GLExtFunctions::GLExtFunctions(QOpenGLContext *pContext)
	{
		glGetTextureHandleARB = reinterpret_cast<PFNGLGETTEXTUREHANDLEARBPROC>(pContext->getProcAddress("glGetTextureHandleARB"));
		glMakeTextureHandleResidentARB = reinterpret_cast<PFNGLMAKETEXTUREHANDLERESIDENTARBPROC>(pContext->getProcAddress("glMakeTextureHandleResidentARB"));
		glMakeTextureHandleNonResidentARB = reinterpret_cast<PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC>(pContext->getProcAddress("glMakeTextureHandleNonResidentARB"));
	}

	bool GLExtFunctions::IsAllFunctionsArePresentedAndSupported() const
	{
		if (!glGetTextureHandleARB) return false;
		if (!glMakeTextureHandleResidentARB) return false;
		if (!glMakeTextureHandleNonResidentARB) return false;

		return true;
	}

	using GLFunctions = QOpenGLFunctions_4_5_Core;

#define ENCODE_MESH_IDX(modelId, meshId) ((static_cast<uint64_t>(modelId) << 32) | static_cast<uint64_t>(meshId))
#define DECODE_MODEL_ID(encoded) (static_cast<uint32_t>((encoded) >> 32))
#define DECODE_MESH_ID(encoded) (static_cast<uint32_t>((encoded) & 0xFFFFFFFF))

	struct MeshInfo
	{
		uint32_t indexOffset = 0;
		uint32_t indexCount = 0;
		gamelib::mat::MATRenderState renderState {
		    "GENERATED",
		    true,
		    true,
		    true,
		    true,
		    false,
		    1.0f,
		    0.f,
		    255,
		    gamelib::mat::MATCullMode::CM_DontCare,
		    gamelib::mat::MATBlendMode::BM_ADD,
		    {}};
	};

	/**
	 * A memory aligned (by 0x10) bbox definition
	 */
	struct BoundingBoxDef
	{
		glm::vec4 vMin { 0.f };
		glm::vec4 vMax { 0.f };

		BoundingBoxDef() = default;
		BoundingBoxDef(const glm::vec3& min, const glm::vec3& max) : vMin(min, 0.f), vMax(max, 0.f) {}
		explicit BoundingBoxDef(const gamelib::BoundingBox& gbb) : vMin(gbb.min, 0.f), vMax(gbb.max, 0.f) {}
		BoundingBoxDef& operator=(const gamelib::BoundingBox& gbb)
		{
			vMin = glm::vec4(gbb.min, 0.f);
			vMax = glm::vec4(gbb.max, 0.f);
			return *this;
		}

		BoundingBoxDef& operator=(gamelib::BoundingBox&& gbb)
		{
			vMin = glm::vec4(gbb.min, 0.f); gbb.min = glm::vec3(0.f);
			vMax = glm::vec4(gbb.max, 0.f); gbb.max = glm::vec3(0.f);
			return *this;
		}

		[[nodiscard]] gamelib::BoundingBox AsBounds() const { return gamelib::BoundingBox(vMin, vMax); }
	};


	/**
	 * @brief This structure holds render data remains to only current game level
	 */
	struct SceneRenderWidget::RenderContext
	{
		RenderContext(GLFunctions* pGLFunctions, GLExtFunctions* pExtFunctions, gamelib::Level* pGameLevel);
		~RenderContext();

		void setup();
		bool buildTextureCache();
		bool buildGeometryBatch();
		bool buildTransformCache();
		void syncTransforms();

		/// --- Data -------------------
		GLFunctions* GL       = nullptr;
		GLExtFunctions* GLExt = nullptr;
		gamelib::Level* Level = nullptr;

		// Materials
		QMap<uint64_t, MeshInfo> Meshes;  // Information about meshes (each PrimId aka Model is a bunch of meshes)
		QMap<int32_t, uint32_t> ModelToMeshesCount; // Contains information about amount of meshes inside model (helper for Meshes storage) ; primId to amount of meshes
		QVector<GLuint> TexturesCache;  // Original textures pool
		QVector<GLuint64> ResidentialTextures;  // A vector if resident texture handles (passed to SSBO)
		QMap<QString, uint32_t> NamedResidentialTextures; // Name to texture index in ResidentialTextures
		QMap<uint32_t, uint32_t> GlacierTextureIndexToResidentialTextureHandle; // Glacier Texture Index to index in ResidentialTextures

		// BoundingBoxes
		QMap<int32_t, gamelib::BoundingBox> BoundingBoxes; // Non transformed & in local space bboxes (vMin & vMax) ; primId to bbox

		// Geometry mega batch
		GLuint MainGeometryVAO = 0;
		GLuint MainGeometryVBO = 0;
		GLuint MainGeometryEBO = 0;
		uint32_t MainGeometryVertexCapacity = 400'000;
		uint32_t MainGeometryIndexCapacity = 400'000;

		// Transforms
		struct ObjectTransformDescription {
			glm::mat4 Matrix    {  1.f };  /// CPU: Write  GPU: Read
			glm::vec4 BoundsMin { -1.f };  /// CPU: Write  GPU: Read
			glm::vec4 BoundsMax {  1.f };  /// CPU: Write  GPU: Read
			glm::vec4 Status    {  0.f };  /// CPU: ReadWrite GPU: Write | X - Is Visible (GPU), Y - PrimitiveID (CPU), Z, W - unused
		};

		QVector<ObjectTransformDescription> Transforms; // Linear memory chunk to store all transforms directly. There are will be copied to GPU SSBO
		QVector<BoundingBoxDef> WorldBoundingBoxes; // A cached world space bounding boxes. Indexing same to Transforms
		QMap<gamelib::scene::SceneObject*, uint32_t> ObjectToTransformIndex;

		uint32_t SSBOMaxCapacity = 0;
		GLuint TransformSSBO = 0;
		GLuint TexturesSSBO = 0;

		// Indirect renderer
		// Indirect buffer contains all draw commands but executed only by DrawGroup's
		GLuint IndirectDrawBuffer = 0;
		uint32_t IndirectDrawCommandsCapacity = 100'000;
		QVector<IndirectRenderDrawCommand> Commands {};
		std::ptrdiff_t IndirectDrawNonTransparentCommands = 0;
		uint32_t IndirectDrawNonTransparentCommandsCount = 0;
		std::ptrdiff_t IndirectDrawTransparentCommands = 0;
		uint32_t IndirectDrawTransparentCommandsCount = 0;
	};

	struct SceneRenderWidget::RenderCommon
	{
		RenderCommon(GLFunctions* pGLFunctions, GLExtFunctions::Ptr&& pGLExtFunctions);
		~RenderCommon();

		void setup();

		/// --- Data -------------------
		enum EUniformID { U_CAMERA_PROJ_VIEW = 0, MAX_UNIFORM_INDEX };

		QSharedPointer<QOpenGLShaderProgram> DefaultShader = nullptr;
		QSharedPointer<QOpenGLShaderProgram> CullingShader = nullptr;
		GLuint DefaultShaderUniformLocations[EUniformID::MAX_UNIFORM_INDEX] { 0 };
		GLuint CullingShaderUniformLocations[EUniformID::MAX_UNIFORM_INDEX] { 0 };

		GLFunctions* GL = nullptr;
		GLExtFunctions::Ptr GLExt = nullptr;
		uint32_t MaxSSBOCapacity = 0;
	};

	SceneRenderWidget::SceneRenderWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
	{
		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(4, 6);
		format.setProfile(QSurfaceFormat::CoreProfile);

#ifdef BMEDIT_DEBUG
		// Enable debug context for debug build
		qDebug() << "[DEBUG BUILD] Scene renderer will enable DebugContext!";

		format.setOption(QSurfaceFormat::DebugContext);
#endif

		setFormat(format);
	}

	SceneRenderWidget::~SceneRenderWidget() noexcept = default;

	void SceneRenderWidget::initializeGL()
	{
		// Build common resources
		m_pCommon = std::make_unique<RenderCommon>(
		    QOpenGLVersionFunctionsFactory::get<GLFunctions>(QOpenGLContext::currentContext()),
			std::make_unique<GLExtFunctions>(QOpenGLContext::currentContext())
		);

		// Setup OpenGL debug context if we've in debug
#ifdef BMEDIT_DEBUG
		m_pCommon->GL->glEnable(GL_DEBUG_OUTPUT);
		m_pCommon->GL->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		m_pCommon->GL->glDebugMessageCallback(BMEdit_OpenGLMessageCallback, nullptr);
		m_pCommon->GL->glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		qDebug() << "[DEBUG BUILD] Attached OpenGL message listener";
#endif

		// Check requirements
		if (!SceneRenderWidget::checkRenderRequirements())
		{
			std::terminate(); // dies here
			return;
		}

		// Check ext
		if (!m_pCommon->GLExt->IsAllFunctionsArePresentedAndSupported())
		{
			QMessageBox::critical(nullptr,
			                      "GPU compatibility failure",
			                      QString("Your GPU does not support one ore more required functions"));

			std::terminate();
			return;
		}

		m_pCommon->setup();

		qDebug() << "Base render stubs are inited";
	}

	void SceneRenderWidget::paintGL()
	{
		if (!m_pLevel) return; // Do nothing when level not presented yet
		if (m_eLoaderState == ELevelLoadState::LLS_FAILED_TO_LOAD) return; // Don't render anything

		if (m_eLoaderState == ELevelLoadState::LLS_READY)
		{
			if (m_bRenderListDirty)
			{
				// Generate new commands
				updateViewLists();
				generateDrawCommands();
			}

			if (m_bTransformsDirty)
			{
				m_pContext->syncTransforms();
				m_bTransformsDirty = false;
			}

			// Perform commands
			drawScene();
		}
		else if (m_eLoaderState == ELevelLoadState::LLS_NONE)
		{
			loadLevelImpl();
		}
	}

	void SceneRenderWidget::resizeGL(int w, int h)
	{
		Q_UNUSED(w)
		Q_UNUSED(h)

		// Update projection
		m_camera.setViewport(w, h);
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

			if ((movementMask & render::CameraMovementMaskValues::CM_FORWARD) && (movementMask & render::CameraMovementMaskValues::CM_BACKWARD))
				movementMask &= ~(render::CameraMovementMaskValues::CM_FORWARD | render::CameraMovementMaskValues::CM_BACKWARD);

			if ((movementMask & render::CameraMovementMaskValues::CM_LEFT) && (movementMask & render::CameraMovementMaskValues::CM_RIGHT))
				movementMask &= ~(render::CameraMovementMaskValues::CM_LEFT | render::CameraMovementMaskValues::CM_RIGHT);

			if (movementMask > 0 && movementMask != (render::CameraMovementMaskValues::CM_SPEEDUP_MOD))
			{
				m_camera.handleKeyboardMovement(movementMask /* dt */);
				m_bRenderListDirty = true; // moved

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
				m_camera.processMouseMovement(xOffset, yOffset /* dt */);

				// Moved
				m_bRenderListDirty = true;
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
			// RayCast
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
			// Drop previously loaded level
			resetLevel();

			// Store a new data
			m_pLevel = pLevel;
			m_bFirstMouseQuery = true;
			m_eLoaderState = ELevelLoadState::LLS_NONE;

			resetViewMode();
		}
	}

	void SceneRenderWidget::resetLevel()
	{
		// drop resources
		m_pLevel = nullptr;
		m_bFirstMouseQuery = true;
		m_pContext = nullptr; // Drop level resources here
		m_eLoaderState = ELevelLoadState::LLS_NONE;

		resetViewMode();
		repaint();
	}

	void SceneRenderWidget::setGeomViewMode(gamelib::scene::SceneObject* sceneObject)
	{
		assert(sceneObject != nullptr);

		// Focus on obj
	}

	void SceneRenderWidget::setWorldViewMode()
	{
		// Unfocus
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

	}

	void SceneRenderWidget::resetSelectedObject()
	{
	}

	void SceneRenderWidget::moveCameraTo(const glm::vec3& position)
	{
		if (!m_pLevel)
			return;

		m_camera.setPosition(position);
		m_bRenderListDirty = true;
		repaint();
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

	void SceneRenderWidget::onRedrawRequested()
	{
		if (m_pLevel)
			repaint();
	}

	void SceneRenderWidget::onObjectMoved(gamelib::scene::SceneObject *sceneObject)
	{
		// Impl
		if (m_pContext)
		{
			if (auto objectToIndexIt = m_pContext->ObjectToTransformIndex.find(sceneObject); objectToIndexIt != m_pContext->ObjectToTransformIndex.end())
			{
				const glm::mat4 mWorld = sceneObject->getWorldTransform();
				m_pContext->Transforms[*objectToIndexIt].Matrix = mWorld;

				const auto primId = GetSceneObjectPrimitiveID(m_pLevel, sceneObject);
				if (primId)
				{
					// Update world bounding boxes
					const auto worldBBox = gamelib::BoundingBox::toWorld(m_pContext->BoundingBoxes[primId], mWorld);
					m_pContext->WorldBoundingBoxes[*objectToIndexIt] = worldBBox;
					m_pContext->Transforms[*objectToIndexIt].BoundsMin = glm::vec4(worldBBox.min, 1.f);
					m_pContext->Transforms[*objectToIndexIt].BoundsMax = glm::vec4(worldBBox.max, 1.f);
				}

				m_bTransformsDirty = true; // NOTE: Maybe we should upload only part?
			}
		}
	}

	bool SceneRenderWidget::checkRenderRequirements() const
	{
		Q_ASSERT(m_pCommon != nullptr);

		// #0: Print debug stuff
#ifdef BMEDIT_DEBUG
		const auto vendor = std::string((const char*)m_pCommon->GL->glGetString(GL_VENDOR));
		const auto renderer = std::string((const char*)m_pCommon->GL->glGetString(GL_RENDERER));
		const auto version = std::string((const char*)m_pCommon->GL->glGetString(GL_VERSION));
		const auto glslVersion = std::string((const char*)m_pCommon->GL->glGetString(GL_SHADING_LANGUAGE_VERSION));

		qDebug() << "OpenGL Vendor: " << vendor;
		qDebug() << "OpenGL Renderer: " << renderer;
		qDebug() << "OpenGL Version: " << version;
		qDebug() << "GLSL Version: " << glslVersion;
#endif

		// #0: Check OpenGL version (at least 4.6 is required)
		GLint glMajorVersion, glMinorVersion;
		m_pCommon->GL->glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
		m_pCommon->GL->glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
		qDebug() << "OpenGL Version: " << QString("%1.%2").arg(glMajorVersion).arg(glMinorVersion);

		if (glMajorVersion < 4 || glMinorVersion < 6)
		{
			QMessageBox::critical(nullptr,
			                      "GPU compatibility failure",
			                      QString("Your GPU does not support OpenGL 4.6, only OpenGL %1.%2 supported")
			                          .arg(glMajorVersion)
			                          .arg(glMinorVersion));
			return false;
		}

		// #1 : Bindless support
		const bool bHasBindlessTextures = QOpenGLContext::currentContext()->hasExtension("GL_ARB_bindless_texture");
		if (!bHasBindlessTextures)
		{
			QMessageBox::critical(nullptr,
			                      "GPU compatibility failure",
			                      QString("Your GPU must support GL_ARB_bindless_texture extension, but it doesn't. Please, contact BMEdit developers about this"));
			return false;
		}

		// #1.1: Check how much textures we've able to use as bindless. Required at least 2000 to fit any level
		// Here the catch: no way to detect is it possible to allocate N residential textures or not. We will try to do it at load level stage
		// #1.2: DSA - supported by OpenGL 4.6 core profile. Don't need to check anything

		// #2: For indirect draw we need to have GL_ARB_draw_indirect at least!
		const bool bHasIndirectDraw = QOpenGLContext::currentContext()->hasExtension("GL_ARB_draw_indirect");

		if (!bHasIndirectDraw)
		{
			QMessageBox::critical(nullptr,
			                      "GPU compatibility failure",
			                      QString("Your GPU does not support indirect rendering. At least OpenGL 4.5 required"));

			return false;
		}

		// #3: For rendering we need to use SSBO
		GLint maxSSBOSize = 0;
		m_pCommon->GL->glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSSBOSize);

		constexpr int kMinSSBOEntries = 128;
		constexpr int kMaxSSBOEntries = 128'000;

		if (maxSSBOSize < kMinSSBOEntries * sizeof(glm::mat4))
		{
			QMessageBox::critical(nullptr, "GPU compatibility failure", "Your GPU has too small SSBO buffer size");
			return false;
		}

		m_pCommon->MaxSSBOCapacity = std::min(kMaxSSBOEntries, static_cast<int>(maxSSBOSize / sizeof(glm::mat4)));
		qDebug() << "GPU: Detected SSBO buffer size is " << maxSSBOSize << " bytes. It will fit at least "
		         << (maxSSBOSize / sizeof(glm::mat4)) << " mat4x4 instances, but we will use only " << m_pCommon->MaxSSBOCapacity;

		return true;
	}

	void SceneRenderWidget::loadLevelImpl()
	{
		Q_ASSERT(m_pLevel != nullptr);  // WTF #1
		Q_ASSERT(m_pCommon != nullptr); // WTF #2
		Q_ASSERT(m_eLoaderState == ELevelLoadState::LLS_NONE);

		if (m_eLoaderState != ELevelLoadState::LLS_NONE)
			return;

		m_eLoaderState = ELevelLoadState::LLS_LOADING;

		if (!m_pContext)
		{
			m_pContext = std::make_unique<RenderContext>(
			    QOpenGLVersionFunctionsFactory::get<GLFunctions>(QOpenGLContext::currentContext()),
			    m_pCommon->GLExt.get(),
			    m_pLevel);

			m_pContext->SSBOMaxCapacity = m_pCommon->MaxSSBOCapacity;
			Q_ASSERT(m_pContext->SSBOMaxCapacity > 0);

			m_pContext->setup();
		}

		Q_ASSERT(m_pContext != nullptr); // WTF #3

		// Ok, now we've ready to upload all textures to GPU
		// Just load all textures and make them residential to use bindless textures during rendering process
		if (!m_pContext->buildTextureCache())
		{
			m_eLoaderState = ELevelLoadState::LLS_FAILED_TO_LOAD;
			QMessageBox::critical(this, "GPU issue", "Unable to create textures cache or make them bindless. Failed to load level, check log for details");
			emit resourceLoadFailed("Unable to create a textures cache for this level");
			return;
		}

		if (!m_pContext->buildGeometryBatch())
		{
			m_eLoaderState = ELevelLoadState::LLS_FAILED_TO_LOAD;
			QMessageBox::critical(this, "GPU issue", "Unable to create geometry batch for this level");
			emit resourceLoadFailed("Unable to create geometry batch for this level");
			return;
		}

		if (!m_pContext->buildTransformCache())
		{
			m_eLoaderState = ELevelLoadState::LLS_FAILED_TO_LOAD;
			QMessageBox::critical(this, "GPU issue", "Unable to create transforms cache");
			emit resourceLoadFailed("Unable to create transforms cache for this level");
			return;
		}

		qDebug() << "Scene resources are ready";

		// Move camera to player position (find ZHitman3 instance on scene, move & rotate camera)
		m_pContext->Level->forEachObjectOfTypeWithInheritance("ZHitman3", [this](const gamelib::scene::SceneObject::Ptr& pPlayerGround) -> bool {
			auto pPlayer = pPlayerGround->getParent().lock();
			Q_ASSERT(pPlayer != nullptr);

			const glm::vec3 vPosition = pPlayer->getPosition();
			const glm::mat4 mMatrix = pPlayer->getWorldTransform();

			glm::vec3 vScale, vTranslation, vSkew;
			glm::vec4 vPerspective;
			glm::quat vOrientation;
			glm::decompose(mMatrix, vScale, vOrientation, vTranslation, vSkew, vPerspective);

			const auto transformIndex = m_pContext->ObjectToTransformIndex[pPlayerGround.get()];
			gamelib::BoundingBox worldBoundingBox = m_pContext->WorldBoundingBoxes[transformIndex].AsBounds();

			auto [fWidth, fDepth, fHeight] = worldBoundingBox.getDimensions();

			// Move camera to position
			const glm::vec3 vCameraPos = pPlayer->getPosition() + glm::vec3(-1.5f * fWidth, 1.2f * fDepth, 0.f);
			m_camera.setPosition(vCameraPos);
			m_camera.setOrientation(vOrientation);

			qDebug() << "Move camera to player position (" << vCameraPos.x << vCameraPos.y << vCameraPos.z << ")";
			return false; // break on first found player
		});

		m_bRenderListDirty = true;
		m_eLoaderState = ELevelLoadState::LLS_READY;
		emit resourcesReady();
	}

	int32_t GetSceneObjectPrimitiveID(const gamelib::Level* pLevel, const gamelib::scene::SceneObject* pObject)
	{
		if (!pLevel) return 0;
		if (!pObject) return 0;

		if (pObject->isInheritedOf("ZItem"))
		{
			// ZItem refs to another scene object
			auto rItemTemplate = pObject->getProperties().getObject<std::string>("rItemTemplate");
			const auto pItemTemplate = pLevel->getSceneObjectByGEOMREF(rItemTemplate);

			if (pItemTemplate)
			{
				gamelib::scene::SceneObject::Ptr pItem = nullptr;

				// Item found by path. That's cool! But this is not an item, for item need to ask Ground... object inside
				for (const auto& rChild : pItemTemplate->getChildren())
				{
					if (auto child = rChild.lock(); child && child->getName().starts_with("Ground"))
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

	int32_t GetSceneObjectPrimitiveID(const gamelib::Level* pLevel, const gamelib::scene::SceneObject::Ptr& pObject)
	{
		return GetSceneObjectPrimitiveID(pLevel, pObject.get());
	}

	bool CanSeeObject(const gamelib::scene::SceneObject* pObject)
	{
		if (!pObject) return false;

#if 0
		using CM = gamelib::gms::ECollisionMask;
		constexpr uint32_t kExpectedToSeeMask = CM::COLIMASK_Sight | CM::COLIMASK_Hero | CM::COLIMASK_NPC | CM::COLIMASK_Background;

		// Check for preset
		if (!(pObject->getGeomInfo().getColiBits() & kExpectedToSeeMask))
			return false;
#endif

		// Check for 'banned' props
		if (pObject->getName() == "AdditionalResources" || pObject->getName() == "AllLevels/mainsceneincludes.zip" || pObject->getName() == "AllLevels/equipment.zip")
			return false;

		return true;
	}

	bool CanSeeObject(const gamelib::scene::SceneObject::Ptr& pObject)
	{
		return CanSeeObject(pObject.get());
	}

	bool IsObjectACollisionArea(const gamelib::scene::SceneObject::Ptr& pObject)
	{
		Q_ASSERT(pObject != nullptr);

		if (auto parent = pObject->getParent().lock(); parent && parent->getType()->getName() == "ZROOM" && parent->getName() == pObject->getName())
		{
			return true;
		}

		return false;
	}

	void SceneRenderWidget::updateViewLists()
	{
		Q_ASSERT(m_pLevel != nullptr);
		Q_ASSERT(m_bRenderListDirty); // expected to have dirty draw list before
		Q_ASSERT(m_pCommon);
		Q_ASSERT(m_pCommon->CullingShader);

		m_pCommon->CullingShader->bind();

		// Enable SSBO
		m_pContext->GL->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pContext->TransformSSBO); // Only transforms

		// Upload camera data
		m_pCommon->DefaultShader->setUniformValue(static_cast<GLint>(m_pCommon->DefaultShaderUniformLocations[RenderCommon::EUniformID::U_CAMERA_PROJ_VIEW]),
		                                          QMatrix4x4(glm::value_ptr(m_camera.getProjView())).transposed());

		GLuint workGroupSize = 64;
		GLuint numGroups = (m_pContext->Transforms.size() + workGroupSize - 1) / workGroupSize;

		m_pCommon->GL->glDispatchCompute(numGroups, 1, 1);
		m_pCommon->GL->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void SceneRenderWidget::generateDrawCommands()
	{
		Q_ASSERT(m_pLevel != nullptr);
		Q_ASSERT(m_bRenderListDirty); // expected to have dirty draw list before

		struct RenderEntity
		{
			uint32_t startIndex { 0 };
			uint32_t indicesCount { 0 };
			uint32_t transformIndex { 0 };
			bool bIsTransparent { false };
			glm::vec3 position { 0.f };
		};

		// Clear commands
		m_pContext->Commands.clear();

		// Collect drawables
		QList<RenderEntity> aNonTransparentObjects {};
		QList<RenderEntity> aTransparentObjects {};
		int iAcceptedEntries = 0;

#if 0
		if (!m_pLevel->getSceneObjects().empty())
		{
			auto DrawVisitor = [this, &aNonTransparentObjects, &aTransparentObjects, &iAcceptedEntries](const gamelib::scene::SceneObject::Ptr& Object) -> gamelib::scene::SceneObject::EVisitResult {
				using VR = gamelib::scene::SceneObject::EVisitResult;

				const bool bInvisible = Object->getProperties().getObject<bool>("Invisible", false);
				const auto vPosition  = Object->getPosition();
				auto primId = GetSceneObjectPrimitiveID(m_pLevel, Object);

				if (const auto& n = Object->getType()->getName(); n == "ZSHADOWMESHOBJ" || n == "ZBOUND" || n == "ZLIGHT" || n == "ZENVIRONMENT" || n == "ZOMNILIGHT" || n == "ZSPOTLIGHT" || n == "ZSPOTLIGHTSQUARE")
				{
					// Do not draw us & our children
					return VR::VR_NEXT;
				}

				if (!m_bIgnoreVisibility)
				{
					if (bInvisible)
					{
						return VR::VR_NEXT;
					}
				}

				// Is it drawable?
				if (!primId)
				{
					return VR::VR_CONTINUE;
				}

				// Check that our 'object' is not a collision box
				if (IsObjectACollisionArea(Object))
				{
					return VR::VR_NEXT; // Do not render collision meshes
				}

				// Check is it visible
				const auto transformIndex = m_pContext->ObjectToTransformIndex[Object.get()];
				gamelib::BoundingBox worldBoundingBox = m_pContext->WorldBoundingBoxes[transformIndex].AsBounds();

				if (!m_camera.canSeeObject(worldBoundingBox))
				{
					// Not in view
					return VR::VR_NEXT;
				}

				// Here we need to store all MESHES, not MODELS
				const auto meshesCount = m_pContext->ModelToMeshesCount[primId];

				for (int32_t i = 0; i < meshesCount; i++)
				{
					if (auto it = m_pContext->Meshes.find(ENCODE_MESH_IDX(primId, i)); it != m_pContext->Meshes.end())
					{
						const bool bIsTransparent = it->renderState.isBlendEnabled();
						QList<RenderEntity>& toInsert = bIsTransparent ? aTransparentObjects : aNonTransparentObjects;

						RenderEntity& renderEntity    = toInsert.emplace_back();
						renderEntity.bIsTransparent   = bIsTransparent;
						renderEntity.startIndex       = it->indexOffset;
						renderEntity.indicesCount     = it->indexCount;
						renderEntity.transformIndex   = transformIndex;

						// It's really weird, but at this point I don't know actual position of this mesh in the world.
						// Anyway, world bounding box is still good point to understand relative to camera position.
						renderEntity.position = worldBoundingBox.getCenter();

						++iAcceptedEntries;
					}
				}

				return VR::VR_CONTINUE;
			};

			m_pLevel->getSceneObjects()[0]->visitChildren(DrawVisitor);
		}
#endif

		m_pCommon->GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_pContext->TransformSSBO);
		auto* ptr = reinterpret_cast<RenderContext::ObjectTransformDescription*>(m_pCommon->GL->glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
		if (ptr)
		{
			for (int i = 0; i < m_pContext->Transforms.size(); ++i)
			{
				auto& objectDescription = ptr[i];
				const bool bIsVisible = static_cast<bool>(objectDescription.Status.x > 0.5f);

				if (!bIsVisible)
				{
					continue;
				}

				// Here we need to store all MESHES, not MODELS
				auto primId = static_cast<int32_t>(objectDescription.Status.y);
				const auto meshesCount = m_pContext->ModelToMeshesCount[primId];

				for (int32_t j = 0; j < meshesCount; j++)
				{
					if (auto it = m_pContext->Meshes.find(ENCODE_MESH_IDX(primId, j)); it != m_pContext->Meshes.end())
					{
						const bool bIsTransparent = it->renderState.isBlendEnabled();
						QList<RenderEntity>& toInsert = bIsTransparent ? aTransparentObjects : aNonTransparentObjects;

						RenderEntity& renderEntity    = toInsert.emplace_back();
						renderEntity.bIsTransparent   = bIsTransparent;
						renderEntity.startIndex       = it->indexOffset;
						renderEntity.indicesCount     = it->indexCount;
						renderEntity.transformIndex   = i; //transformIndex;

						// It's really weird, but at this point I don't know actual position of this mesh in the world.
						// Anyway, world bounding box is still good point to understand relative to camera position.
						renderEntity.position = gamelib::BoundingBox(objectDescription.BoundsMin, objectDescription.BoundsMax).getCenter();

						++iAcceptedEntries;
					}
				}
			}

			m_pCommon->GL->glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else
		{
			qWarning() << "Failed to map SSBO TransformSSBO";
			return;
		}

		// Sor
		auto SortPred = [this](const RenderEntity& a, const RenderEntity& b)
		{
			const float fADistanceToCamera = glm::length(m_camera.getPosition() - a.position);
			const float fBDistanceToCamera = glm::length(m_camera.getPosition() - b.position);

			return fADistanceToCamera > fBDistanceToCamera;
		};

		std::sort(aTransparentObjects.begin(), aTransparentObjects.end(), SortPred);
		std::sort(aNonTransparentObjects.begin(), aNonTransparentObjects.end(), SortPred);

		// Generate draw commands
		// Non Transparent
		m_pContext->IndirectDrawNonTransparentCommands = static_cast<std::ptrdiff_t>(m_pContext->Commands.size());
		m_pContext->IndirectDrawNonTransparentCommandsCount = static_cast<uint32_t>(aNonTransparentObjects.size());
		for (const auto& renderEntity : aNonTransparentObjects)
		{
			auto& drawCommand = m_pContext->Commands.emplace_back();
			drawCommand.count = renderEntity.indicesCount;
			drawCommand.instanceCount = 1;
			drawCommand.firstIndex = renderEntity.startIndex;
			drawCommand.baseVertex = 0;
			drawCommand.baseInstance = renderEntity.transformIndex;
		}

		// Transparent
		m_pContext->IndirectDrawTransparentCommands = static_cast<std::ptrdiff_t>(m_pContext->Commands.size());
		m_pContext->IndirectDrawTransparentCommandsCount = static_cast<uint32_t>(aTransparentObjects.size());
		for (const auto& renderEntity : aTransparentObjects)
		{
			auto& drawCommand = m_pContext->Commands.emplace_back();
			drawCommand.count = renderEntity.indicesCount;
			drawCommand.instanceCount = 1;
			drawCommand.firstIndex = renderEntity.startIndex;
			drawCommand.baseVertex = 0;
			drawCommand.baseInstance = renderEntity.transformIndex;
		}

		// Send commands to GPU
		m_pContext->GL->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_pContext->IndirectDrawBuffer);

		if (m_pContext->Commands.size() > m_pContext->IndirectDrawCommandsCapacity)
		{
			qDebug() << "Increase Indirect draw buffer size from " << m_pContext->IndirectDrawCommandsCapacity << " to " << m_pContext->Commands.size();
			m_pContext->IndirectDrawCommandsCapacity = m_pContext->Commands.size();
			m_pContext->GL->glBufferData(GL_DRAW_INDIRECT_BUFFER, static_cast<GLsizeiptr>(m_pContext->Commands.size() * sizeof(IndirectRenderDrawCommand)), m_pContext->Commands.data(), GL_DYNAMIC_DRAW);
		}
		else
		{
			m_pContext->GL->glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, static_cast<GLsizeiptr>(m_pContext->Commands.size() * sizeof(IndirectRenderDrawCommand)), m_pContext->Commands.data());
		}

		m_pContext->GL->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		// Finished
		m_bRenderListDirty = false;
	}

	void SceneRenderWidget::drawScene()
	{
		Q_ASSERT(m_pLevel != nullptr);
		Q_ASSERT(!m_bRenderListDirty);

		if (m_pContext->Commands.isEmpty() || (!m_pContext->IndirectDrawTransparentCommandsCount && !m_pContext->IndirectDrawNonTransparentCommandsCount))
			 return; // Do nothing

		// Set viewport
		m_pCommon->GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_pCommon->GL->glClearColor(0.f, 0.f, 0.15f, 1.f);
		m_pCommon->GL->glViewport(0, 0, width(), height());
		m_pCommon->GL->glEnable(GL_DEPTH_TEST);

		// Wireframe (for debug)
		//m_pCommon->GL->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Enable shader
		m_pCommon->DefaultShader->bind();

		// Enable VAO
		m_pContext->GL->glBindVertexArray(m_pContext->MainGeometryVAO);

		// Enable SSBO
		m_pContext->GL->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pContext->TransformSSBO); // Store transforms at #0 slot
		m_pContext->GL->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_pContext->TexturesSSBO);  // Store textures at #1 slot

		// Upload camera data
		m_pCommon->DefaultShader->setUniformValue(static_cast<GLint>(m_pCommon->DefaultShaderUniformLocations[RenderCommon::EUniformID::U_CAMERA_PROJ_VIEW]),
		                                          QMatrix4x4(glm::value_ptr(m_camera.getProjView())).transposed());

		// Enable indirect
		m_pContext->GL->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_pContext->IndirectDrawBuffer);

		// Stage #1: Compute shader calculate visibility list & generate commands buffer
		// Stage #2: Pre-Z pass culling
		// TODO: Impl me
		// Stage #3: Non-Transparent commands
		if (m_pContext->IndirectDrawNonTransparentCommandsCount > 0)
		{
			m_pContext->GL->glDepthMask(GL_TRUE);
			m_pContext->GL->glDisable(GL_BLEND);

			m_pContext->GL->glMultiDrawElementsIndirect(GL_TRIANGLES,
			                                            GL_UNSIGNED_INT,
			                                            (const void *) (m_pContext->IndirectDrawNonTransparentCommands * sizeof(IndirectRenderDrawCommand)),
			                                            static_cast<GLint>(m_pContext->IndirectDrawNonTransparentCommandsCount),
			                                            0 /* stride */);
		}
		// Stage #4: Gizmo (not implemented yet)
		// Stage #5: Transparent commands
		if (m_pContext->IndirectDrawTransparentCommandsCount)
		{
			m_pContext->GL->glDepthMask(GL_TRUE);
			m_pContext->GL->glEnable(GL_BLEND);
			m_pContext->GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			m_pContext->GL->glMultiDrawElementsIndirect(GL_TRIANGLES,
			                                            GL_UNSIGNED_INT,
			                                            (const void *) (m_pContext->IndirectDrawTransparentCommands * sizeof(IndirectRenderDrawCommand)),
			                                            static_cast<GLint>(m_pContext->IndirectDrawTransparentCommandsCount),
			                                            0 /* stride */);
		}
	}

	/// ------------------------------ RenderContext
	SceneRenderWidget::RenderContext::RenderContext(GLFunctions* pGLFunctions, GLExtFunctions* pExtFunctions, gamelib::Level* pGameLevel)
	    : GL(pGLFunctions), Level(pGameLevel), GLExt(pExtFunctions)
	{
	}

	SceneRenderWidget::RenderContext::~RenderContext()
	{
		GL->glDeleteBuffers(1, &MainGeometryVBO);
		MainGeometryVBO = 0;

		GL->glDeleteBuffers(1, &MainGeometryEBO);
		MainGeometryEBO = 0;

		GL->glDeleteVertexArrays(1, &MainGeometryVAO);
		MainGeometryVAO = 0;

		GL->glDeleteBuffers(1, &TransformSSBO);
		TransformSSBO = 0;

		GL->glDeleteBuffers(1, &TexturesSSBO);
		TexturesSSBO = 0;

		GL->glDeleteBuffers(1, &IndirectDrawBuffer);
		IndirectDrawBuffer=  0;

		if (!TexturesCache.empty())
		{
			GL->glDeleteTextures(static_cast<GLsizei>(TexturesCache.size()), TexturesCache.data());
			TexturesCache.clear();
		}
	}

	void SceneRenderWidget::RenderContext::setup()
	{
		// Create mega batch
		GL->glGenVertexArrays(1, &MainGeometryVAO);
		GL->glGenBuffers(1, &MainGeometryVBO);
		GL->glGenBuffers(1, &MainGeometryEBO);

		// Setup VAO
		GL->glBindVertexArray(MainGeometryVAO);

		// Setup VBO
		GL->glBindBuffer(GL_ARRAY_BUFFER, MainGeometryVBO);
		GL->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(MainGeometryVertexCapacity * sizeof(render::GlacierVertex)), nullptr, GL_DYNAMIC_DRAW);

		// Setup EBO
		GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MainGeometryEBO);
		GL->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(MainGeometryIndexCapacity * sizeof(unsigned int)), nullptr, GL_DYNAMIC_DRAW);

		// Enable vertex attributes
		GL->glEnableVertexAttribArray(0);
		GL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(render::GlacierVertex), nullptr); // Position

		GL->glEnableVertexAttribArray(1);
		GL->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(render::GlacierVertex), (void*)(sizeof(float) * 3)); // UV

		GL->glEnableVertexAttribArray(2);
		GL->glVertexAttribIPointer(2, 2, GL_UNSIGNED_INT, sizeof(render::GlacierVertex), (void*)(sizeof(float) * 5)); // Texture Index

		GL->glBindVertexArray(0);

		// Transform SSBO
		GL->glGenBuffers(1, &TransformSSBO);
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, TransformSSBO);
		GL->glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(SSBOMaxCapacity * sizeof(ObjectTransformDescription)), nullptr, GL_DYNAMIC_DRAW);
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Texture SSBO
		GL->glGenBuffers(1, &TexturesSSBO);
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, TexturesSSBO);
		GL->glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(SSBOMaxCapacity * sizeof(uint64_t)), nullptr, GL_DYNAMIC_DRAW);
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Indirect renderer
		GL->glGenBuffers(1, &IndirectDrawBuffer);
		GL->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectDrawBuffer);
		GL->glBufferData(GL_DRAW_INDIRECT_BUFFER, static_cast<GLsizeiptr>(IndirectDrawCommandsCapacity * sizeof(IndirectRenderDrawCommand)), nullptr, GL_DYNAMIC_DRAW);
		GL->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		// Reserve commands list
		Commands.reserve(IndirectDrawCommandsCapacity);

		// Checks
		Q_ASSERT(MainGeometryVAO != 0);
		Q_ASSERT(MainGeometryVBO != 0);
		Q_ASSERT(MainGeometryEBO != 0);
		Q_ASSERT(TransformSSBO != 0);
		Q_ASSERT(TexturesSSBO != 0);
		Q_ASSERT(IndirectDrawBuffer != 0);
		Q_ASSERT(IndirectDrawCommandsCapacity > 0);
	}

	bool SceneRenderWidget::RenderContext::buildTextureCache()
	{
		Q_ASSERT(TexturesCache.empty());

		const auto& Textures = Level->getSceneTextures()->entries;
		if (Textures.empty())
		{
			return false; // Cuz no empty TEX allowed to be here!
		}

		// Create cache
		TexturesCache.resize(static_cast<qsizetype>(Textures.size()));
		ResidentialTextures.reserve(static_cast<qsizetype>(Textures.size()));

		// Create textures
		GL->glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(Textures.size()), TexturesCache.data());

		int iTextureIndex = 0;
		for (const auto& Texture : Textures)
		{
			// Decompress texture & store image
			uint16_t w{0}, h{0};
			std::unique_ptr<std::uint8_t[]> decompressedMemBlk = editor::TextureProcessor::decompressRGBA(Texture, w, h, 0);
			if (!decompressedMemBlk)
			{
				qWarning() << "Failed to decompress texture #" << Texture.m_index << "(" << Texture.m_width << ";" << Texture.m_height << ")";
				Q_ASSERT(false);
				return false;
			}

			// Get current index
			const int iCurrentTexture = iTextureIndex;
			++iTextureIndex;

			const auto texID = TexturesCache[iCurrentTexture];

			GL->glTextureStorage2D(texID, 1, GL_RGBA8, w, h);
			GL->glTextureSubImage2D(texID, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, decompressedMemBlk.get());
			GL->glGenerateTextureMipmap(texID);
			GL->glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			GL->glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GL->glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			GL->glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Make texture residential
			GLuint64 handle = GLExt->glGetTextureHandleARB(TexturesCache[iCurrentTexture]);
			GLExt->glMakeTextureHandleResidentARB(handle);

			// Store into SSBO
			ResidentialTextures.emplace_back(handle);

			// Store cache
			if (Texture.m_fileName.has_value())
			{
				// Store as named too
				NamedResidentialTextures[QString::fromStdString(Texture.m_fileName.value())] = static_cast<uint32_t>(ResidentialTextures.size() - 1);
			}

			// Store glacier based cache
			GlacierTextureIndexToResidentialTextureHandle[Texture.m_index] = static_cast<uint32_t>(ResidentialTextures.size() - 1);
		}

		// Fill textures cache
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, TexturesSSBO);
		GL->glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(sizeof(uint64_t) * ResidentialTextures.size()), ResidentialTextures.data());
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return true;
	}

	bool SceneRenderWidget::RenderContext::buildGeometryBatch()
	{
		// It's more hard task: we need to store ALL geometry entries into 1 single geometry batch, normalize vertices & fix UVs
		Q_ASSERT(Level != nullptr);
		Q_ASSERT(Level->getLevelGeometry() != nullptr);

		// Activate geometry buffer (cuz we will upload geometry here)
		GL->glBindVertexArray(MainGeometryVAO);
		GL->glBindBuffer(GL_ARRAY_BUFFER, MainGeometryVBO);
		GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MainGeometryEBO);

		// Global vertices & indices pool
		QVector<render::GlacierVertex> aVertices {};
		QVector<uint32_t> aIndices {};

		for (const auto& model : Level->getLevelGeometry()->primitives.models)
		{
			// Store primId to meshes count
			ModelToMeshesCount.insert(static_cast<int32_t>(model.chunk), static_cast<uint32_t>(model.meshes.size()));

			// Then check meshes availability
			if (model.meshes.empty())
			{
				// create null model
				qWarning() << "Failed to load model of chunk " << model.chunk << ". Reason: no meshes (empty model)";
				continue;
			}

			// Store bbox
			BoundingBoxes.insert(static_cast<int32_t>(model.chunk), gamelib::BoundingBox(model.boundingBox.vMin, model.boundingBox.vMax));

			// Here we need to allocate space for our 'mesh'
			int32_t meshIdx = 0;

			// Cuz I did it a few times at least
			struct DontForgetToIncrementAValue
			{
				int32_t* pInt = nullptr;

				explicit DontForgetToIncrementAValue(int32_t* ptr) : pInt(ptr) {}

				~DontForgetToIncrementAValue() {
					Q_ASSERT(pInt != nullptr);
					(*pInt)++;
				}
			};


			for (const auto& mesh : model.meshes)
			{
				DontForgetToIncrementAValue dummy { &meshIdx }; // Please, don't forget it!

				if (mesh.vertices.empty())
				{
					// create empty mesh
					qWarning() << "Failed to load mesh at chunk " << model.chunk << ". Reason: no meshes (empty model)";
					continue;
				}

				// Detect mesh texture
				uint32_t meshTexture = 0u;
				bool bTextureResolved = false;

				const auto meshMaterialId = mesh.material_id;
				const auto meshTextureId = mesh.textureId;

				MeshInfo& meshInfo = *Meshes.insert(ENCODE_MESH_IDX(static_cast<int32_t>(model.chunk), meshIdx), {});

				if (meshMaterialId > 0)
				{
					// It has own material
					// Use material (for meshes)
					// First of all we need to know that 'shadows' and other things must be filtered here
					const auto& instances = Level->getLevelMaterials()->materialInstances;
					const auto& classes = Level->getLevelMaterials()->materialClasses;
					const auto& matInstance = instances[mesh.material_id - 1];

					// Store material based data
					{
						for (const auto& binder : matInstance.getBinders())
						{
							bool bRenderStateSaved = false;

							for (const auto& state : binder.renderStates)
							{
								if (!state.isEnabled())
									continue;

								meshInfo.renderState = state;
								bRenderStateSaved = true;
								break;
							}

							if (bRenderStateSaved)
								break;
						}
					}

					// Here we need to find 'color' texture. In most cases we able to use matDiffuse as color texture
					for (const auto& binder : matInstance.getBinders())
					{
						if (bTextureResolved)
							break;

						for (const auto& texture : binder.textures)
						{
							if (bTextureResolved)
								break;

							switch (texture.getPresentedTextureSources())
							{
								case gamelib::mat::PresentedTextureSource::PTS_NOTHING:
									break;  // Nothing

								case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_ID:
								{
									// Only texture id
								    if (auto it = GlacierTextureIndexToResidentialTextureHandle.find(texture.getTextureId()); it != GlacierTextureIndexToResidentialTextureHandle.end())
									{
									    meshTexture = (*it) + 1;
										bTextureResolved = true;
										break;
									}

									qWarning() << "Material refs to texture " << texture.getTextureId() << " but it's not found in cache!";
								}
								break;

								case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_PATH:
								{
									// Only path
									if (auto it = NamedResidentialTextures.find(QString::fromStdString(texture.getTexturePath())); it != NamedResidentialTextures.end())
									{
									    meshTexture = (*it) + 1;
										bTextureResolved = true;
										break;
									}

									qWarning() << "Material refs to texture by path " << texture.getTexturePath() << " but it's not found in cache!";
								}
								break;

								default:
								{
									// Bad case! Undefined behaviour!
									Q_ASSERT_X(false, __FILE__, "Impossible case!");
									break;
								}
							}
						}
					}
				}

				if (!bTextureResolved && meshTextureId > 0)
				{
					// It's ZWINPIC or UI stuff
					// Use texture here (for sprites). Need to find that texture in loaded textures list
					if (auto it = GlacierTextureIndexToResidentialTextureHandle.find(mesh.textureId); it != GlacierTextureIndexToResidentialTextureHandle.end())
					{
						meshTexture = (*it) + 1;
						bTextureResolved = true;
					}
					else
					{
						qWarning() << "No texture found by texture index " << mesh.textureId;
					}
				}
				// otherwise no texture and no need to normalize that texture somehow

				// Save base vertex
				auto baseVertex = aVertices.size();

				if (!bTextureResolved)
				{
					if (meshMaterialId > 0)
					{
						const auto& instances = Level->getLevelMaterials()->materialInstances;
						const auto& classes = Level->getLevelMaterials()->materialClasses;
						const auto& matInstance = instances[mesh.material_id - 1];

						qWarning() << "For Prim " << model.chunk << " not resolved texture reference. MaterialRef = " << meshMaterialId << "(Name: " << matInstance.getName() << "Parent: " << matInstance.getParentName() << ") TextureRef = " << meshTextureId;
					}
					else
					{
						qWarning() << "For Prim " << model.chunk << " not resolved texture reference. MaterialRef = " << meshMaterialId << "TextureRef = " << meshTextureId;
					}
				}

				// Compose geometry
				for (int i = 0; i < mesh.vertices.size(); i++)
				{
					auto& vertex = aVertices.emplace_back();
					vertex.vPos = mesh.vertices[i];
					vertex.iTexIndex = meshTexture;

					if (mesh.uvs.empty())
					{
						vertex.vUV = glm::vec2(.0f);
					}
					else
					{
						vertex.vUV = mesh.uvs[i];
					}
				}

				// Save base index
				auto baseIndex = aIndices.size();

				// Calculate indices
				if (!mesh.indices.empty())
				{
					// Indexed geometry should be converted to global indexed geometry
					meshInfo.indexOffset = baseIndex;
					meshInfo.indexCount = mesh.indices.size() * 3;

					for (const auto& [a, b, c] : mesh.indices)
					{
						aIndices.emplace_back(baseVertex + static_cast<uint32_t>(a));
						aIndices.emplace_back(baseVertex + static_cast<uint32_t>(b));
						aIndices.emplace_back(baseVertex + static_cast<uint32_t>(c));
					}
				}
				else
				{
					// Non-indexed geometry should be converted to indexed
					Q_ASSERT(mesh.vertices.size() % 3 == 0);

					meshInfo.indexOffset = baseIndex;
					meshInfo.indexCount = mesh.vertices.size();

					for (size_t i = 0; i < mesh.vertices.size(); ++i)
					{
						aIndices.emplace_back(baseVertex + static_cast<uint32_t>(i));
					}
				}
			}
		}

		// Upload geometry
		auto ResizeOrUpdateGLBuffer = [this](GLenum target, uint32_t currentSize, uint32_t& capacity, size_t elementSize, const void* data)
		{
			if (currentSize > capacity)
			{
				capacity = currentSize;
				GL->glBufferData(target, static_cast<GLsizeiptr>(elementSize * capacity), data, GL_DYNAMIC_DRAW);
			}
			else
			{
				GL->glBufferSubData(target, 0, static_cast<GLsizeiptr>(elementSize * currentSize), data);
			}
		};

		// Upload
		ResizeOrUpdateGLBuffer(GL_ARRAY_BUFFER, aVertices.size(), MainGeometryVertexCapacity, sizeof(render::GlacierVertex), aVertices.data());
		ResizeOrUpdateGLBuffer(GL_ELEMENT_ARRAY_BUFFER, aIndices.size(), MainGeometryIndexCapacity, sizeof(uint32_t), aIndices.data());

		qDebug() << "Latest gl error: " << GL->glGetError();
		qDebug() << "Total vertices usage: " << aVertices.size();
		qDebug() << "Total indices usage: " << aIndices.size();

		// Unbind
		GL->glBindVertexArray(0);
		GL->glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return true;
	}

	bool SceneRenderWidget::RenderContext::buildTransformCache()
	{
		// Here we need to visit whole scene object by object (in hierarchy order)
		Q_ASSERT(Level != nullptr);
		Q_ASSERT(!Level->getSceneObjects().empty());

		auto Visitor = [this](const gamelib::scene::SceneObject::Ptr& pObject) -> gamelib::scene::SceneObject::EVisitResult {
			using VR = gamelib::scene::SceneObject::EVisitResult;
			const auto primId = GetSceneObjectPrimitiveID(Level, pObject);

			// Store transform & association
			const glm::mat4 mWorld = pObject->getWorldTransform();
			ObjectTransformDescription& transformDescription = Transforms.emplace_back();
			transformDescription.Matrix = mWorld;
			transformDescription.Status.x = 0.f;
			transformDescription.Status.y = static_cast<float>(primId);
			transformDescription.Status.z = transformDescription.Status.w = 0.f;

			// Store bounding box
			gamelib::BoundingBox worldBBox = primId ? gamelib::BoundingBox::toWorld(BoundingBoxes[primId], mWorld) : gamelib::BoundingBox();
			WorldBoundingBoxes.emplace_back(worldBBox); // Store empty bbox if no primId presented at all

			// Store bounds
			transformDescription.BoundsMin = glm::vec4(worldBBox.min, 1.f);
			transformDescription.BoundsMax = glm::vec4(worldBBox.max, 1.f);

			// Store identity
			ObjectToTransformIndex[pObject.get()] = (Transforms.size() - 1);

			// Go deeper cuz in few cases we have situation when "drawable inside drawable"
			return VR::VR_CONTINUE;
		};

		Level->getSceneObjects()[0]->visitChildren(Visitor);
		if (Transforms.isEmpty())
			return false;

		// Upload transforms to GPU
		syncTransforms();
		return true;
	}

	void SceneRenderWidget::RenderContext::syncTransforms()
	{
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, TransformSSBO);
		GL->glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(sizeof(ObjectTransformDescription) * Transforms.size()), Transforms.data());
		GL->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	/// ------------------------------ RenderCommon
	SceneRenderWidget::RenderCommon::RenderCommon(widgets::GLFunctions *pGLFunctions, GLExtFunctions::Ptr&& pGLExtFunctions)
	    : GL(pGLFunctions), GLExt(std::move(pGLExtFunctions))
	{
	}

	SceneRenderWidget::RenderCommon::~RenderCommon()
	{
	}

	void SceneRenderWidget::RenderCommon::setup()
	{
		auto GetContents = [](const QString& path) -> QString {
			QFile file(path);
			file.open(QIODevice::ReadOnly);

			auto res = file.readAll();
			file.close();

			return res;
		};

		auto CollectUniforms = [](uint32_t* cache, QOpenGLShaderProgram* pShaderProg)
		{
			cache[EUniformID::U_CAMERA_PROJ_VIEW] = pShaderProg->uniformLocation("cameraProjView");

			qDebug() << "Collected";
		};

		{
			DefaultShader.reset(new QOpenGLShaderProgram(nullptr));
			DefaultShader->addShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Vertex, GetContents(":/bmedit/mtl_textured_gl33.vsh"));
			DefaultShader->addShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Fragment, GetContents(":/bmedit/mtl_textured_gl33.fsh"));
			if (!DefaultShader->link())
			{
				QMessageBox::critical(nullptr, "Render error", "Failed to link DefaultShader!");
				return;
			}

			// Make cache
			CollectUniforms(&DefaultShaderUniformLocations[0], DefaultShader.get());
		}

		{
		    CullingShader.reset(new QOpenGLShaderProgram(nullptr));
			CullingShader->addShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Compute, GetContents(":/bmedit/culling.csh"));
			if (!CullingShader->link())
			{
				QMessageBox::critical(nullptr, "Render error", "Failed to link CullingShader!");
				return;
			}

			// Make cache
			CollectUniforms(&CullingShaderUniformLocations[0], CullingShader.get());
		}

		qDebug() << "GPU: Shaders are ready";
	}
}