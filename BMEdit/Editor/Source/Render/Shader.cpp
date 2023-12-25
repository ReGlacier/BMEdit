#include <Render/Shader.h>
#include <glm/gtc/type_ptr.hpp>


namespace render
{

	void Shader::discard(QOpenGLFunctions_3_3_Core *gapi)
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

	void Shader::bind(QOpenGLFunctions_3_3_Core *gapi)
	{
		if (programId != kInvalidResource)
		{
			gapi->glUseProgram(programId);
		}
	}

	void Shader::unbind(QOpenGLFunctions_3_3_Core *gapi)
	{
		gapi->glUseProgram(0);
	}

	bool Shader::compile(QOpenGLFunctions_3_3_Core *gapi, const std::string &vertexProgram, const std::string &fragmentProgram, std::string &error)
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

	bool Shader::compileUnit(QOpenGLFunctions_3_3_Core *gapi, GLuint unitId, GLenum unitType, const std::string &unitSource, std::string &error)
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

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, float s)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform1f(location, s);
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, std::int32_t s)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform1i(location, s);
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec2& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform2fv(location, 1, glm::value_ptr(v));
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::ivec2& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform2iv(location, 1, glm::value_ptr(v));
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec3& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform3fv(location, 1, glm::value_ptr(v));
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec4& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniform4fv(location, 1, glm::value_ptr(v));
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat3& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(v));
	}

	void Shader::setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat4& v)
	{
		GLint location = resolveLocation(gapi, id);
		if (location == -1)
			return;

		gapi->glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(v));
	}

	GLint Shader::resolveLocation(QOpenGLFunctions_3_3_Core* gapi, const std::string& id)
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
}