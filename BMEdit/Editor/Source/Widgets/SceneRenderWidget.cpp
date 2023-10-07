#include <Widgets/SceneRenderWidget.h>
#include <Editor/TextureProcessor.h>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLContext>
#include <QDebug>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GameLib/TEX/TEXEntry.h>


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
			size_t textureId { 0 };
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
					qDebug() << "Failed to link shader program: " << std::string_view(&linkLog[0], length);

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
					qDebug() << "Failed to compile shader program: " << std::string_view(&compileLog[0], length);

					error = std::string(&compileLog[0], length);
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

		GLResources() {}
		~GLResources() {}

		void discard(QOpenGLFunctions_3_3_Core* gapi)
		{
			{
				for (auto& texture : m_textures)
				{
					texture.discard(gapi);
				}

				m_textures.clear();
			}

			{
				for (auto& shader : m_shaders)
				{
					shader.discard(gapi);
				}

				m_shaders.clear();
			}

			{
				for (auto& model : m_models)
				{
					model.discardAll(gapi);
				}

				m_models.clear();
			}
		}

		bool hasResources() const
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

		funcs->glClear(GL_COLOR_BUFFER_BIT);
		funcs->glClearColor(0.15f, 0.2f, 0.45f, 1.0f);
		funcs->glEnable(GL_CULL_FACE);
		funcs->glCullFace(GL_BACK);

		// Update projection
		if (m_bDirtyProj)
		{
			updateProjectionMatrix(QWidget::width(), QWidget::height());
		}

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
			    doRenderScene(funcs);
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
			constexpr float kSpeedUp = 100.f;

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
		}
	}

	void SceneRenderWidget::resetLevel()
	{
		if (m_pLevel != nullptr)
		{
			m_pLevel = nullptr;
			m_bFirstMouseQuery = true;
		}
	}

	void SceneRenderWidget::moveCameraTo(const glm::vec3& position)
	{
		if (!m_pLevel)
			return;

		m_camera.setPosition(position);
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

			glFunctions->glGenTextures(1, &newTexture.texture);
			glFunctions->glBindTexture(GL_TEXTURE_2D, newTexture.texture);
			glFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTexture.width, newTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, decompressedMemBlk.get());

			glFunctions->glGenerateMipmap(GL_TEXTURE_2D);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFunctions->glBindTexture(GL_TEXTURE_2D, 0);

			m_resources->m_textures.emplace_back(newTexture);
		}

		qDebug() << "All textures (" << m_pLevel->getSceneTextures()->entries.size() << ") are loaded and ready to be used";
		m_eState = ELevelState::LS_LOAD_GEOMETRY;
	}

	void SceneRenderWidget::doLoadGeometry(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		int kDebugTextureTextureIndex = 0;

		// Select debug texture
		static constexpr const char* kGlacierMissingTex = "_Glacier/Missing_01";
		static constexpr const char* kWorldColiTex = "_TEST/Worldcoli";

		for (int i = 0; i < m_pLevel->getSceneTextures()->entries.size(); i++)
		{
			const gamelib::tex::TEXEntry& texture = m_pLevel->getSceneTextures()->entries[i];

			if (texture.m_fileName.has_value() && (texture.m_fileName.value() == kGlacierMissingTex || texture.m_fileName.value() == kWorldColiTex))
			{
				kDebugTextureTextureIndex = i;
				break;
			}
		}

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

				for (const auto& index : mesh.indices)
				{
					indices.emplace_back(index.a);
					indices.emplace_back(index.b);
					indices.emplace_back(index.c);
				}

				// And upload it to GPU
				GLResources::Mesh& glMesh = glModel.meshes.emplace_back();
				glMesh.trianglesCount = static_cast<int>(mesh.indices.size() / 3);

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
				glFunctions->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glFunctions->glEnableVertexAttribArray(0);

				glFunctions->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
				glFunctions->glEnableVertexAttribArray(1);

				// TODO: Precache texture id (color channel)
				glMesh.textureId = kDebugTextureTextureIndex; // mesh.textureId != 0 ? mesh.textureId : 133;

				// Next mesh
				++meshIdx;
			}
		}

		qDebug() << "All models (" << m_pLevel->getLevelGeometry()->primitives.models.size() << ") are loaded & ready to use!";
		m_eState = ELevelState::LS_COMPILE_SHADERS;
	}

	void SceneRenderWidget::doCompileShaders(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// TODO: In future we will use shaders from FS, but now I need to debug this stuff easier
		static constexpr const char* kVertexShader = R"(
#version 330 core

// Layout
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

// Common
struct Camera
{
    mat4 projView;
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
    gl_Position = i_uCamera.projView * i_uTransform.model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    g_TexCoord = aUV;
}
)";

		static constexpr const char* kFragmentShader = R"(
#version 330 core

uniform sampler2D iActiveTexture;
in vec2 g_TexCoord;

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = texture(iActiveTexture, g_TexCoord);
}
)";

		// TODO: Compile shader
		std::string compileError;
		GLResources::Shader defaultShader;
		if (!defaultShader.compile(glFunctions, kVertexShader, kFragmentShader, compileError))
		{
			m_pLevel = nullptr;
			m_eState = ELevelState::LS_NONE;

			emit resourceLoadFailed(QString("Failed to compile shaders: %1").arg(QString::fromStdString(compileError)));
			return;
		}

		m_resources->m_shaders.emplace_back(defaultShader);

		qDebug() << "Shaders (" << m_resources->m_shaders.size() << ") compiled and ready to use!";
		m_eState = ELevelState::LS_RESET_CAMERA_STATE;
	}

	void SceneRenderWidget::doResetCameraState(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// Take scene and find first camera (ZCAMERA instance I guess)
		glm::vec3 cameraPosition { .0f };

		for (const auto& sceneObject : m_pLevel->getSceneObjects())
		{
			if (sceneObject->getType()->getName() == "ZCAMERA")
			{
				// Nice, camera found!
				cameraPosition.x = sceneObject->getProperties()["Position"][1].getOperand().get<float>();
				cameraPosition.y = sceneObject->getProperties()["Position"][2].getOperand().get<float>();
				cameraPosition.z = sceneObject->getProperties()["Position"][3].getOperand().get<float>();
				break;
			}
		}

		m_camera.setPosition(cameraPosition); // TODO: Need teleport camera to player and put under player's head

		emit resourcesReady();

		m_eState = ELevelState::LS_READY; // Done!
	}

	void SceneRenderWidget::doRenderScene(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		LEVEL_SAFE_CHECK()

		// Out ROOT is always first object. Start tree hierarchy visit from ROOT
		const gamelib::scene::SceneObject::Ptr& root = m_pLevel->getSceneObjects()[0];

		doRenderGeom(glFunctions, root);
	}

	void SceneRenderWidget::discardResources(QOpenGLFunctions_3_3_Core* glFunctions)
	{
		if (m_resources)
		{
			m_resources->discard(glFunctions);
		}

		m_eState = ELevelState::LS_NONE; // Switch to none, everything is gone
	}

	void propertyMatrixToGlmMatrix(const gamelib::Span<gamelib::prp::PRPInstruction>& matrix, glm::mat4& result)
	{
		result = glm::mat4(1.f);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				// +1 cuz #0 - begin array opcode
				result[i][j] = matrix[(1 + (j * 3 + i))].getOperand().get<float>();
			}
		}
	}

	void SceneRenderWidget::doRenderGeom(QOpenGLFunctions_3_3_Core* glFunctions, const gamelib::scene::SceneObject::Ptr& geom) // NOLINT(*-no-recursion)
	{
		// First of all we need to take primitive id and visibillity flag
		const std::int32_t primId = geom->getProperties()["PrimId"][0].getOperand().get<std::int32_t>();
		const bool bIsVisible = !geom->getProperties().hasProperty("Invisible") || geom->getProperties()["Invisible"][0].getOperand().get<bool>();
		const glm::vec3 vPosition = glm::vec3 {
		    geom->getProperties()["Position"][1].getOperand().get<float>(),
		    geom->getProperties()["Position"][2].getOperand().get<float>(),
		    geom->getProperties()["Position"][3].getOperand().get<float>()
		};

		if (primId != 0)
		{
			// Do render?
			if (!bIsVisible)
				return; // Don't draw children of invisible ZSTDOBJ

			// Extract matrix from properties
			glm::mat4 modelMatrix = glm::mat4(1.f);
			modelMatrix = glm::translate(modelMatrix, vPosition);
			//propertyMatrixToGlmMatrix(geom->getProperties()["Matrix"], modelMatrix);

			// And bind required resources
			// TODO: Optimize me
			const auto& models = m_resources->m_models;
			auto modelIt = std::find_if(models.begin(), models.end(),
			    [&primId](const GLResources::Model& model) -> bool {
				    return model.chunkId == (primId - 1);
			    });

			if (modelIt != models.end())
			{
				const GLResources::Model& model = *modelIt;

				// Render all meshes
				for (const auto& mesh : model.meshes)
				{
					// Render single mesh
					// 1. Activate default shader
					m_resources->m_shaders[0].bind(glFunctions);

					// 2. Submit uniforms
					m_resources->m_shaders[0].setUniform(glFunctions, "i_uTransform.model", modelMatrix);
					m_resources->m_shaders[0].setUniform(glFunctions, "i_uCamera.projView", m_matProjection * m_camera.getViewMatrix());

					// 3. Bind texture
					glFunctions->glBindTexture(GL_TEXTURE_2D, m_resources->m_textures[mesh.textureId].texture);

					// 3. Activate VAO
					glFunctions->glBindVertexArray(mesh.vao);

					// 4. Submit
					if (mesh.ibo != GLResources::kInvalidResource)
					{
						// Draw indexed
						glFunctions->glDrawElements(GL_TRIANGLES, mesh.trianglesCount, GL_UNSIGNED_SHORT, nullptr);
					}
					else
					{
						// Draw elements
						glFunctions->glDrawArrays(GL_TRIANGLES, 0, mesh.trianglesCount);
					}

					// ... End
					glFunctions->glBindTexture(GL_TEXTURE_2D, 0);
					m_resources->m_shaders[0].unbind(glFunctions);
				}
			}
			// otherwise draw red bbox!
		}

		// Render others
		for (const auto& childRef : geom->getChildren())
		{
			if (auto child = childRef.lock())
			{
				doRenderGeom(glFunctions, child);
			}
		}
	}
}