#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <Render/GLResource.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>


namespace render
{
	struct Shader
	{
		GLuint vertexProgramId   { kInvalidResource };
		GLuint fragmentProgramId { kInvalidResource };
		GLuint programId { kInvalidResource };

		Shader() = default;

		void discard(QOpenGLFunctions_3_3_Core* gapi);

		void bind(QOpenGLFunctions_3_3_Core* gapi);

		void unbind(QOpenGLFunctions_3_3_Core* gapi);

		bool compile(QOpenGLFunctions_3_3_Core* gapi, const std::string& vertexProgram, const std::string& fragmentProgram, std::string& error);

		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, float s);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, std::int32_t s);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec2& v);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::ivec2& v);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec3& v);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::vec4& v);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat3& v);
		void setUniform(QOpenGLFunctions_3_3_Core* gapi, const std::string& id, const glm::mat4& v);
		GLint resolveLocation(QOpenGLFunctions_3_3_Core* gapi, const std::string& id);

	private:
		bool compileUnit(QOpenGLFunctions_3_3_Core* gapi, GLuint unitId, GLenum unitType, const std::string& unitSource, std::string& error);

	private:
		std::map<std::string, GLint> m_locationsCache;
	};
}