#include <BMRender/ShaderProgram.h>
#include <glad/glad.h>
#include <cassert>
#include <vector>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <BMRender/RenderGL.h>


namespace bmr
{
	class UpdateShaderFallbackData
	{
		int *m_fallbackShaderIdPtr { nullptr };
		int m_fallbackShaderId { -1 };
		ShaderSources *m_fallbackSourcesPtr { nullptr };
		ShaderSources m_fallbackSources { };
		bool m_revertChanges { true };

	public:
		UpdateShaderFallbackData(int *idPtr, ShaderSources *srcsPtr) : m_fallbackShaderIdPtr(idPtr), m_fallbackSourcesPtr(srcsPtr)
		{
			m_fallbackShaderId = *m_fallbackShaderIdPtr;
			m_fallbackSources = *m_fallbackSourcesPtr;
		}

		~UpdateShaderFallbackData()
		{
			if (m_revertChanges)
			{
				*m_fallbackShaderIdPtr = m_fallbackShaderId;
				*m_fallbackSourcesPtr = m_fallbackSources;
				m_revertChanges = false;
			}
		}

		UpdateShaderFallbackData(const UpdateShaderFallbackData&) = delete;
		UpdateShaderFallbackData& operator=(const UpdateShaderFallbackData&) = delete;

		void disableFallback()
		{
			m_revertChanges = false;
		}

		[[nodiscard]] int getShaderId() const
		{
			return m_fallbackShaderId;
		}
	};


	ShaderProgram::ShaderProgram() = default;

	ShaderProgram::~ShaderProgram()
	{
		if (m_id != kInvalidId)
		{
			glDeleteProgram(m_id); BM_RENDER_GL_CHECK_STATE()
			m_id = kInvalidId;
		}
	}

	CompileResult ShaderProgram::compile(ShaderSources &&sources)
	{
		if (!sources.contains(ShaderSourceCodeType::SRC_VERTEX) && !sources.contains(ShaderSourceCodeType::SRC_FRAGMENT))
		{
			return CompileResult { "SRC_VERTEX & SRC_FRAGMENT source codes must be presented!" };
		}

		m_sources = std::move(sources);

		CompileResult compileResult {};
		int shaderProgramId = glCreateProgram(); BM_RENDER_GL_CHECK_STATE()
		bool isCompiled = true;

		std::vector<int> compiledSubPrograms;
		compiledSubPrograms.reserve(8);

		auto compileEntry = [](ShaderSourceCodeType entryType, const std::string &sourceCode, CompileResult &compileResult, const std::set<ShaderDefinition>& defs) -> int
		{
			GLenum shaderType = GL_NONE;

			switch (entryType)
			{
				case ShaderSourceCodeType::SRC_VERTEX:   shaderType = GL_VERTEX_SHADER; break;
				case ShaderSourceCodeType::SRC_FRAGMENT: shaderType = GL_FRAGMENT_SHADER; break;
			}

			int shaderId = glCreateShader(shaderType); BM_RENDER_GL_CHECK_STATE()

			std::stringstream definitionsStream;
			definitionsStream << "\n\n";

			for (const auto& def: defs)
			{
				definitionsStream << "#define " << def << " //!AUTO!\n";
			}

			std::string definitions = definitionsStream.str();
			std::string finalSourceCode = sourceCode;

			auto versionIt = finalSourceCode.find("#version");
			auto newLineIt = finalSourceCode.find_first_of('\n', versionIt);

			finalSourceCode.insert(newLineIt, definitions);

			const GLchar* glSrc = reinterpret_cast<const GLchar*>(finalSourceCode.c_str());
			const GLint glLen = static_cast<int>(finalSourceCode.length());

			glShaderSource(shaderId, 1, &glSrc, &glLen); BM_RENDER_GL_CHECK_STATE()
			glCompileShader(shaderId); BM_RENDER_GL_CHECK_STATE()

			GLint isCompiled = 0;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled); BM_RENDER_GL_CHECK_STATE()

			if (isCompiled == GL_FALSE)
			{
				constexpr int kCompileLogSize = 512;

				compileResult.isCompiled = false;

				char compileLog[kCompileLogSize] = { 0 };
				GLint length { 0 };

				glGetShaderInfoLog(shaderId, kCompileLogSize, &length, &compileLog[0]); BM_RENDER_GL_CHECK_STATE()
				glDeleteShader(shaderId); BM_RENDER_GL_CHECK_STATE()

				compileResult.errorMessage.resize(length);
				std::memcpy(compileResult.errorMessage.data(), &compileLog[0], length);

				return -1;
			}

			return shaderId;
		};

		for (const auto& [srcType, src]: m_sources)
		{
			int shaderSubProgId = compileEntry(srcType, src, compileResult, m_definitions);
			if (shaderSubProgId < 0)
			{
				isCompiled = false;
				break;
			}

			compiledSubPrograms.push_back(shaderSubProgId);
			glAttachShader(shaderProgramId, shaderSubProgId); BM_RENDER_GL_CHECK_STATE()
		}

		if (!isCompiled)
		{
			for (const auto& shaderSubProgId: compiledSubPrograms)
			{
				glDeleteShader(shaderSubProgId); BM_RENDER_GL_CHECK_STATE()
			}

			glDeleteProgram(shaderProgramId); BM_RENDER_GL_CHECK_STATE()
		}
		else
		{
			glLinkProgram(shaderProgramId); BM_RENDER_GL_CHECK_STATE()
			GLint isLinkOk = GL_FALSE;

			glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &isLinkOk); BM_RENDER_GL_CHECK_STATE()

			if (isLinkOk != GL_FALSE)
			{
				m_id = shaderProgramId;
			}
		}

		m_needRebuild = false; // drop flag anyway
		return compileResult;
	}

	bool ShaderProgram::enable()
	{
		if (m_isInUse)
		{
			return false;
		}

		if (m_needRebuild)
		{
			m_needRebuild = false;

			UpdateShaderFallbackData fallback { &m_id, &m_sources };

			if (!compile(std::move(m_sources)))
			{
				return false;
			}

			fallback.disableFallback();
			glDeleteProgram(fallback.getShaderId()); BM_RENDER_GL_CHECK_STATE()
		}

		glUseProgram(m_id); BM_RENDER_GL_CHECK_STATE()
		m_isInUse = true;
		return true;
	}

	void ShaderProgram::disable()
	{
		if (m_isInUse)
		{
			glUseProgram(0); BM_RENDER_GL_CHECK_STATE()
			m_isInUse = false;
		}
	}

	struct ShaderParameterSetterVisitor
	{
		int location { -1 };

		void operator()(std::int32_t value) const
		{
			glUniform1i(location, value); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(std::uint32_t value) const
		{
			glUniform1ui(location, value); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(float value) const
		{
			glUniform1f(location, value); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::vec2& value) const
		{
			glUniform2fv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::vec3& value) const
		{
			glUniform3fv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::vec4& value) const
		{
			glUniform4fv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::ivec2& value) const
		{
			glUniform2iv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::ivec3& value) const
		{
			glUniform3iv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::ivec4& value) const
		{
			glUniform4iv(location, 1, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::mat3& value) const
		{
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}

		void operator()(const glm::mat4& value) const
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); BM_RENDER_GL_CHECK_STATE()
		}
	};

	void ShaderProgram::setParameter(const std::string &parameterName, ShaderParameter &&parameter)
	{
		int location = -1;
		if (auto it = m_parameters.find(parameterName); it != m_parameters.end())
		{
			location = it->second.location;
		}

		if (location < 0)
		{
			location = glGetUniformLocation(m_id, parameterName.c_str()); BM_RENDER_GL_CHECK_STATE()
			if (location == -1)
			{
				assert(false && "Location not found");
				return;
			}

			m_parameters[parameterName] = { location };
		}

		ShaderParameterSetterVisitor visitor { location };
		std::visit(visitor, parameter);
	}

	void ShaderProgram::addDefinition(const ShaderDefinition &definition)
	{
		if (!m_definitions.contains(definition))
		{
			m_needRebuild = true;
			m_definitions.insert(definition);
		}
	}

	void ShaderProgram::removeDefinition(const ShaderDefinition &definition)
	{
		if (m_definitions.contains(definition))
		{
			m_needRebuild = true;
			m_definitions.erase(definition);
		}
	}

	void ShaderProgram::clearDefinitions()
	{
		m_definitions.clear();
		m_needRebuild = true;
	}

	bool ShaderProgram::hasDefinition(const ShaderDefinition &definition) const
	{
		return m_definitions.contains(definition);
	}
}