#include <Widgets/SceneRenderWidget.h>
#include <Editor/TextureProcessor.h>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLContext>
#include <QDebug>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <optional>


namespace widgets
{
	struct SceneRenderWidget::GLResources
	{
		static constexpr GLuint kInvalidResource = 0xFDEADC0D;

		struct Mesh
		{
			GLuint vao { kInvalidResource };
			GLuint vbo { kInvalidResource };
			GLuint ibo { kInvalidResource };
			GLuint glTextureId { kInvalidResource }; /// Render OpenGL texture resource handle

			uint16_t materialId { 0 }; /// Id of material from Glacier mesh (just copy)

			int trianglesCount { 0 };

			void discard(QOpenGLFunctions_3_3_Core* gapi)
			{
				if (vao != kInvalidResource)
				{
					gapi->glDeleteVertexArrays(1, &vao);
					vao = kInvalidResource;
				}

				if (vbo != kInvalidResource)
				{
					gapi->glDeleteBuffers(1, &vbo);
					vbo = kInvalidResource;
				}

				if (ibo != kInvalidResource)
				{
					gapi->glDeleteBuffers(1, &ibo);
					ibo = kInvalidResource;
				}
			}
		};

		struct Model
		{
			std::vector<Mesh> meshes {};
			std::uint32_t chunkId { 0 };

			void discardAll(QOpenGLFunctions_3_3_Core* gapi)
			{
				for (auto& mesh : meshes)
				{
					mesh.discard(gapi);
				}

				meshes.clear();
			}
		};

		struct Texture
		{
			uint16_t width { 0 };
			uint16_t height { 0 };
			GLuint texture { kInvalidResource };
			std::optional<std::uint32_t> index {}; /// Index of texture from TEX container
			std::optional<std::string> texPath {}; /// [Optional] Path to texture in TEX container (path may not be defined in TEX!)

			void discard(QOpenGLFunctions_3_3_Core* gapi)
			{
				width = height = 0;

				if (texture != kInvalidResource)
				{
					gapi->glDeleteTextures(1, &texture);
				}
			}
		};

		struct Shader
		{
			GLuint vertexProgramId   { kInvalidResource };
			GLuint fragmentProgramId { kInvalidResource };
			GLuint programId { kInvalidResource };

			Shader() = default;

			void discard(QOpenGLFunctions_3_3_Core* gapi)
			{
				if (vertexProgramId != kInvalidResource)
				{
					gapi->glDeleteShader(vertexProgramId);
					vertexProgramId = kInvalidResource;
				}

				if (fragmentProgramId != kInvalidResource)
				{
					gapi->glDeleteShader(fragmentProgramId);
					fragmentProgramId = kInvalidResource;
				}

				if (programId != kInvalidResource)
				{
					gapi->glDeleteShader(programId);
					programId = kInvalidResource;
				}
			}

			void bind(QOpenGLFunctions_3_3_Core* gapi)
			{
				if (programId != kInvalidResource)
				{
					gapi->glUseProgram(programId);
				}
			}

			void unbind(QOpenGLFunctions_3_3_Core* gapi)
			{
				gapi->glUseProgram(0);
			}

			bool compile(QOpenGLFunctions_3_3_Core* gapi, const std::string& vertexProgram, const std::string& fragmentProgram, std::string& error)
			{
				// Allocate root program
				programId = gapi->glCreateProgram();
				vertexProgramId = gapi->glCreateShader(GL_VERTEX_SHADER);
				fragmentProgramId = gapi->glCreateShader(GL_FRAGMENT_SHADER);

				// Compile vertex program
				if (!compileUnit(gapi, vertexProgramId, GL_VERTEX_SHADER, vertexProgram, error))
				{
					qDebug() << "Failed to compile vertex shader program";
					assert(false && "Failed to compile vertex shader program");
					return false;
				}

				gapi->glAttachShader(programId, vertexProgramId);

				// Compile fragment program
				if (!compileUnit(gapi, fragmentProgramId, GL_FRAGMENT_SHADER, fragmentProgram, error))
				{
					qDebug() << "Failed to compile vertex shader program";
					assert(false && "Failed to compile fragment shader program");
					return false;
				}

				gapi->glAttachShader(programId, fragmentProgramId);

				// Linking
				gapi->glLinkProgram(programId);

				// Check linking status
				GLint linkingIsOK = GL_FALSE;

				gapi->glGetProgramiv(programId, GL_LINK_STATUS, &linkingIsOK);
				if (linkingIsOK == GL_FALSE)
				{
					constexpr int kCompileLogSize = 512;

					char linkLog[kCompileLogSize] = { 0 };
					GLint length { 0 };

					gapi->glGetProgramInfoLog(programId, kCompileLogSize, &length, &linkLog[0]);

					error = std::string(&linkLog[0], length);
					qDebug() << "Failed to link shader program: " << QString::fromStdString(error);

					assert(false);
					return false;
				}

				// Done
				return true;
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, float s)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform1f(location, s);
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, std::int32_t s)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform1i(location, s);
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec2& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform2fv(location, 1, glm::value_ptr(v));
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::ivec2& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform2iv(location, 1, glm::value_ptr(v));
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec3& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform3fv(location, 1, glm::value_ptr(v));
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec4& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniform4fv(location, 1, glm::value_ptr(v));
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat3& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(v));
			}

			void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat4& v)
			{
				GLint location = resolveLocation(gapi, id);
				if (location == -1)
					return;

				gapi->glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(v));
			}

			GLint resolveLocation(QOpenGLFunctions_3_3_Core* gapi, const std::string& id) const
			{
				if (auto it = m_locationsCache.find(id); it == m_locationsCache.end())
				{
					GLint result = gapi->glGetUniformLocation(programId, id.c_str());
					m_locationsCache[id] = result;
					return result;
				}
				else
				{
					return it->second;
				}
			}

		private:
			bool compileUnit(QOpenGLFunctions_3_3_Core* gapi, GLuint unitId, GLenum unitType, const std::string& unitSource, std::string& error)
			{
				const GLchar* glSrc = reinterpret_cast<const GLchar*>(unitSource.c_str());
				const GLint glLen = static_cast<int>(unitSource.length());

				gapi->glShaderSource(unitId, 1, &glSrc, &glLen);
				gapi->glCompileShader(unitId);

				GLint isCompiled = 0;
				gapi->glGetShaderiv(unitId, GL_COMPILE_STATUS, &isCompiled);

				if (isCompiled == GL_FALSE)
				{
					constexpr int kCompileLogSize = 512;

					char compileLog[kCompileLogSize] = { 0 };
					GLint length { 0 };

					gapi->glGetShaderInfoLog(unitId, kCompileLogSize, &length, &compileLog[0]);

					error = std::string(&compileLog[0], length);
					qDebug() << "Failed to compile shader program: " << QString::fromStdString(error);

					assert(false && "Failed to compile unit!");
					return false;
				}

				return true;
			}

		private:
			mutable std::map<std::string, GLint> m_locationsCache;
		};

		std::vector<Texture> m_textures {};
		std::vector<Shader> m_shaders {};
		std::vector<Model> m_models {};
		std::unordered_map<uint32_t, size_t> m_modelsCache {};  /// primitive index to model index in m_models
		GLuint m_iGLDebugTexture { 0 };
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
					model.discardAll(gapi);
				}

				m_models.clear();
			}

			// Empty cache
			m_modelsCache.clear();

			// Release refs
			m_iGLDebugTexture = 0u;
			m_iTexturedShaderIdx = 0u;
			m_iGizmoShaderIdx = 0u;
		}

		[[nodiscard]] bool hasResources() const
		{
			return !m_textures.empty() || !m_shaders.empty() || !m_models.empty();
		}
	};

	struct GlacierVertex
	{
		glm::vec3 vPos {};
		glm::vec2 vUV {};
	};

	SceneRenderWidget::SceneRenderWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
	{
	}

	SceneRenderWidget::~SceneRenderWidget() noexcept = default;

	void SceneRenderWidget::initializeGL()
	{
		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setStencilBufferSize(8);
		format.setVersion(3, 3);
		format.setProfile(QSurfaceFormat::CoreProfile);
		setFormat(format);

		// Create resource holder
		m_resources = std::make_unique<GLResources>();
	}

	void SceneRenderWidget::paintGL()
	{
		auto funcs = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
		if (!funcs) {
			qFatal("Could not obtain required OpenGL context version");
			return;
		}

		// Begin frame
		funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		funcs->glClearColor(0.15f, 0.2f, 0.45f, 1.0f);

		// Z-Buffer testing
		funcs->glEnable(GL_DEPTH_TEST);
		funcs->glDepthFunc(GL_LESS);

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
			    if (m_eViewMode == EViewMode::VM_WORLD_VIEW)
			    {
					doRenderScene(funcs);
				}
			    else if (m_eViewMode == EViewMode::VM_GEOM_PREVIEW)
			    {
				    if (m_pSceneObjectToView)
				    {
						// Render object & ignore Invisible flag
						doRenderGeom(funcs, m_pSceneObjectToView, true);
					}
			    }
		    }
		    break;
		}
	}

	void SceneRenderWidget::resizeGL(int w, int h)
	{
		Q_UNUSED(w);
		Q_UNUSED(h);

		// Will recalc on next frame
		m_bDirtyProj = true;
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
				m_camera.processKeyboard(renderer::Camera_Movement::FORWARD, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_S)
			{
				m_camera.processKeyboard(renderer::Camera_Movement::BACKWARD, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_A)
			{
				m_camera.processKeyboard(renderer::Camera_Movement::LEFT, kBaseDt, kSpeedUp);
				bMoved = true;
			}
			else if (event->key() == Qt::Key_D)
			{
				m_camera.processKeyboard(renderer::Camera_Movement::RIGHT, kBaseDt, kSpeedUp);
				bMoved = true;
			}

			if (bMoved)
				repaint();
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
			float xpos = event->pos().x();
			float ypos = event->pos().y();

			if (m_bFirstMouseQuery)
			{
				m_mouseLastPosition = event->pos();
				m_bFirstMouseQuery = false;
				return;
			}

			float xoffset = xpos - m_mouseLastPosition.x();
			float yoffset = m_mouseLastPosition.y() - ypos;

			m_mouseLastPosition = event->pos();

			// Update camera
			qDebug() << "Mouse moved (" << xoffset << ";" << yoffset << ")";
			m_camera.processMouseMovement(xoffset, yoffset);
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
	}

	void SceneRenderWidget::updateProjectionMatrix(int w, int h)
	{
		m_matProjection = glm::perspectiveFov(glm::radians(m_fFOV), static_cast<float>(w), static_cast<float>(h), m_fZNear, m_fZFar);
		m_bDirtyProj = false;
	}

	void SceneRenderWidget::setLevel(gamelib::Level *pLevel)
	{
		if (m_pLevel != pLevel)
		{
			m_eState = ELevelState::LS_NONE;
			m_pLevel = pLevel;
			m_bFirstMouseQuery = true;
			resetViewMode();
			resetRenderMode();
		}
	}

	void SceneRenderWidget::resetLevel()
	{
		if (m_pLevel != nullptr)
		{
			m_pLevel = nullptr;
			m_bFirstMouseQuery = true;
			resetViewMode();
			resetRenderMode();
		}
	}

	void SceneRenderWidget::setGeomViewMode(gamelib::scene::SceneObject* sceneObject)
	{
		assert(sceneObject != nullptr);

		if (sceneObject)
		{
			m_eViewMode = EViewMode::VM_GEOM_PREVIEW;
			m_pSceneObjectToView = sceneObject;
			m_camera.setPosition(glm::vec3(0.f));
			repaint();
		}
	}

	void SceneRenderWidget::setWorldViewMode()
	{
		m_eViewMode = EViewMode::VM_WORLD_VIEW;
		m_pSceneObjectToView = nullptr;
		m_camera.setPosition(glm::vec3(0.f));
		repaint();
	}

	void SceneRenderWidget::resetViewMode()
	{
		setWorldViewMode();
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
			GLResources::Texture newTexture;

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

		qDebug() << "All textures (" << m_pLevel->getSceneTextures()->entries.size() << ") are loaded and ready to be used";
		m_eState = ELevelState::LS_LOAD_GEOMETRY;
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doLoadGeometry(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// To avoid of future problems we've allocating null model at slot #0 and assign to it chunk #0
		m_resources->m_models.emplace_back().chunkId = 0;

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

			GLResources::Model& glModel = m_resources->m_models.emplace_back();
			glModel.chunkId = model.chunk;

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
				GLResources::Mesh& glMesh = glModel.meshes.emplace_back();
				glMesh.trianglesCount = mesh.trianglesCount;

				// Allocate VAO, VBO & IBO stuff
				glFunctions->glGenVertexArrays(1, &glMesh.vao);
				glFunctions->glGenBuffers(1, &glMesh.vbo);
				if (!indices.empty())
				{
					glFunctions->glGenBuffers(1, &glMesh.ibo);
				}

				// Attach VAO
				glFunctions->glBindVertexArray(glMesh.vao);
				glFunctions->glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);

				// Upload vertices
				glFunctions->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlacierVertex), &vertices[0], GL_STATIC_DRAW);

				// Upload indices
				if (!indices.empty())
				{
					glFunctions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ibo);
					glFunctions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint16_t), &indices[0], GL_STATIC_DRAW);
				}

				// Setup vertex format
				const GLintptr vertexCoordinateOffset = 0 * sizeof(float);
				const GLintptr UVCoordinateOffset = 3 * sizeof(float);
				const GLsizei stride = 5 * sizeof(float);

				glFunctions->glEnableVertexAttribArray(0);
				glFunctions->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)vertexCoordinateOffset);

				glFunctions->glEnableVertexAttribArray(1);
				glFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)UVCoordinateOffset);

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
						glMesh.glTextureId = GLResources::kInvalidResource;
					}
					else if (parentName == "Bad")
					{
						// Use 'bad' debug texture
						glMesh.glTextureId = m_resources->m_iGLDebugTexture;
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
										glMesh.glTextureId = m_resources->m_iGLDebugTexture;
									}

									// But mark us as 'found'
									bTextureFound = true;

									// Done
									break;
								}
							}
						}

						if (matInstance.getBinders().empty())
						{
							// use debug texture
							glMesh.glTextureId = m_resources->m_iGLDebugTexture;
						}
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

		// TODO: In future we will use shaders from FS, but now I need to debug this stuff easier
		static constexpr const char* kTexturedVertexShader = R"(
#version 330 core

// Layout
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

// Common
struct Camera
{
    mat4  proj;
    mat4  view;
	ivec2 resolution;
};

struct Transform
{
    mat4 model;
};

// Uniforms
uniform Camera i_uCamera;
uniform Transform i_uTransform;

// Out
out vec2 g_TexCoord;

void main()
{
    gl_Position = i_uCamera.proj * i_uCamera.view * i_uTransform.model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    g_TexCoord = aUV;
}
)";

		static constexpr const char* kTexturedFragmentShader = R"(
#version 330 core

uniform sampler2D i_ActiveTexture;
in vec2 g_TexCoord;

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = texture(i_ActiveTexture, g_TexCoord);
}
)";

		static constexpr const char* kGizmoVertexShader = R"(
#version 330 core

// Layout
layout (location = 0) in vec3 aPos;

// Common
struct Camera
{
    mat4  proj;
    mat4  view;
	ivec2 resolution;
};

struct Transform
{
    mat4 model;
};

// Uniforms
uniform Camera i_uCamera;
uniform Transform i_uTransform;

void main()
{
    gl_Position = i_uCamera.proj * i_uCamera.view * i_uTransform.model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

		static constexpr const char* kGizmoFragmentShader = R"(
#version 330 core

uniform vec4 i_Color;

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = i_Color;
}
)";

		// Compile shaders
		std::string compileError;
		{
			GLResources::Shader texturedShader;
			if (!texturedShader.compile(glFunctions, kTexturedVertexShader, kTexturedFragmentShader, compileError))
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
			GLResources::Shader gizmoShader;
			if (!gizmoShader.compile(glFunctions, kGizmoVertexShader, kGizmoFragmentShader, compileError))
			{
				m_pLevel = nullptr;
				m_eState = ELevelState::LS_NONE;

				emit resourceLoadFailed(QString("Failed to compile shaders (gizmo): %1").arg(QString::fromStdString(compileError)));
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

		// Take scene and find first camera (ZCAMERA instance I guess)
		glm::vec3 cameraPosition { .0f };

		for (const auto& sceneObject : m_pLevel->getSceneObjects())
		{
			// Need to fix this code. Most 'ZCAMERA' objects refs to 2D scene view, need to find another better way to find 'start' camera.
			// At least we can try to find ZPlayer/ZHitman3 object to put camera near to player
			if (sceneObject->getType()->getName() == "ZCAMERA")
			{
				// Nice, camera found!
				cameraPosition = sceneObject->getProperties().getObject<glm::vec3>("Position", glm::vec3(.0f));
				break;
			}
		}

		m_camera.setPosition(cameraPosition); // TODO: Need teleport camera to player and put under player's head

		emit resourcesReady();

		m_eState = ELevelState::LS_READY; // Done!
		repaint(); // call to force jump into next state
	}

	void SceneRenderWidget::doRenderScene(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// Update projection
		if (m_bDirtyProj)
		{
			updateProjectionMatrix(QWidget::width(), QWidget::height());
		}

		// First of all we need to find ZBackdrop and render scene from this geom
		m_pLevel->forEachObjectOfType("ZBackdrop", [this, glFunctions](const gamelib::scene::SceneObject::Ptr& sceneObject) -> bool {
			doRenderGeom(glFunctions, sceneObject.get());

			// Render only 1 ZBackdrop
			return false;
		});

		// Then we need to find our 'current room'
		// How to find current room? Idk, let's find all 'rooms'?
		std::vector<gamelib::scene::SceneObject::Ptr> roomsToRender;
		roomsToRender.reserve(16);

		m_pLevel->forEachObjectOfTypeWithInheritance("ZROOM", [&roomsToRender](const gamelib::scene::SceneObject::Ptr& sceneObject) -> bool {
			if (sceneObject->getName() != "ROOT" && sceneObject->getType()->getName() != "ZBackdrop")
			{
				// Save room
				roomsToRender.emplace_back(sceneObject);
			}

			// Render every room
			return true;
		});

		// Ok, we have rooms to draw, let's draw 'em all
		for (const auto& room : roomsToRender)
		{
			doRenderGeom(glFunctions, room.get());
		}
	}

	void SceneRenderWidget::discardResources(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		if (m_resources)
		{
			m_resources->discard(glFunctions);
		}

		m_eState = ELevelState::LS_NONE; // Switch to none, everything is gone
	}

	void SceneRenderWidget::doRenderGeom(QOpenGLFunctions_3_3_Core* glFunctions, const gamelib::scene::SceneObject* geom, bool bIgnoreVisibility) // NOLINT(*-no-recursion)
	{
		// Save "scene" resolution
		const glm::ivec2 viewResolution { QWidget::width(), QWidget::height() };

		// Take params
		const auto primId     = geom->getProperties().getObject<std::int32_t>("PrimId", 0);
		const bool bInvisible = geom->getProperties().getObject<bool>("Invisible", false);
		const auto vPosition  = geom->getProperties().getObject<glm::vec3>("Position", glm::vec3(0.f));
		const auto mMatrix    = geom->getProperties().getObject<glm::mat3>("Matrix", glm::mat3(1.f));

		// Don't draw invisible things
		if (bInvisible && !bIgnoreVisibility)
			return;

		// TODO: Check for culling here (object visible or not)

		// Check that object could be rendered by any way
		if (primId != 0)
		{
			// Extract matrix from properties
			const glm::mat4 transformMatrix = glm::mat4(glm::transpose(mMatrix));
			const glm::mat4 translateMatrix = glm::translate(glm::mat4(1.f), vPosition);

			// Am I right here?
			const glm::mat4 modelMatrix = translateMatrix * transformMatrix;

			// RenderGod
			if (m_resources->m_modelsCache.contains(primId))
			{
				const GLResources::Model& model = m_resources->m_models[m_resources->m_modelsCache[primId]];

				// Render all meshes
				for (const auto& mesh : model.meshes)
				{
					// Render single mesh
					// 0. Check that we able to draw it
					if (mesh.glTextureId == GLResources::kInvalidResource)
					{
						// Draw "error" bounding box
						// And continue
						continue;
					}

					// 1. Activate default shader
					GLResources::Shader& texturedShader = m_resources->m_shaders[m_resources->m_iTexturedShaderIdx];

					texturedShader.bind(glFunctions);

					// 2. Submit uniforms
					texturedShader.setUniform(glFunctions, "i_uTransform.model", modelMatrix);
					texturedShader.setUniform(glFunctions, "i_uCamera.proj", m_matProjection);
					texturedShader.setUniform(glFunctions, "i_uCamera.view", m_camera.getViewMatrix());
					texturedShader.setUniform(glFunctions, "i_uCamera.resolution", viewResolution);

					// 3. Bind texture
					if (mesh.glTextureId != GLResources::kInvalidResource)
					{
						glFunctions->glBindTexture(GL_TEXTURE_2D, mesh.glTextureId);
					}

					// 3. Activate VAO
					glFunctions->glBindVertexArray(mesh.vao);

					auto doSubmitGPUCommands = [glFunctions, mesh]() {
						if (mesh.ibo != GLResources::kInvalidResource)
						{
							// Draw indexed
							glFunctions->glDrawElements(GL_TRIANGLES, (mesh.trianglesCount * 3), GL_UNSIGNED_SHORT, nullptr);
						}
						else
						{
							// Draw elements
							glFunctions->glDrawArrays(GL_TRIANGLES, 0, mesh.trianglesCount);
						}
					};

					if (m_renderMode & RenderMode::RM_TEXTURE)
					{
						// normal draw
						doSubmitGPUCommands();
					}

					if (m_renderMode & RenderMode::RM_WIREFRAME)
					{
						glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						doSubmitGPUCommands();
						// reset back
						glFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}

					// 5. Draw bounding box

					// 6. Unbind texture and shader (expected to switch between materials, but not now)
					glFunctions->glBindTexture(GL_TEXTURE_2D, 0);
					m_resources->m_shaders[0].unbind(glFunctions);
				}
			}
			// otherwise draw red bbox!
		}

		// Draw children
		for (const auto& childRef : geom->getChildren())
		{
			if (auto child = childRef.lock())
			{
				doRenderGeom(glFunctions, child.get());
			}
		}
	}
}