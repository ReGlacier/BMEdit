#pragma once

#include <cstdint>
#include <variant>
#include <string>
#include <map>
#include <set>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>


namespace bmr
{
	using ShaderParameter = std::variant<std::int32_t, std::uint32_t, float,
	                                     glm::mat4, glm::mat3,
	                                     glm::vec2, glm::ivec2,
	                                     glm::vec3, glm::ivec3,
	                                     glm::vec4, glm::ivec4>;
	using ShaderDefinition = std::string;

	enum class ShaderSourceCodeType : std::uint8_t
	{
		SRC_VERTEX,   ///< Vertex program
		SRC_FRAGMENT  ///< Fragment program
	};

	using ShaderSources = std::map<ShaderSourceCodeType, std::string>;

	struct CompileResult
	{
		bool isCompiled { false };
		std::string errorMessage {};

		CompileResult() : isCompiled(true), errorMessage() {}
		explicit CompileResult(const std::string& msg) : isCompiled(false), errorMessage(msg) {}

		explicit operator bool() const { return isCompiled; }
	};

	class ShaderProgram
	{
		static constexpr int kInvalidId = -1;

	public:
		ShaderProgram();
		~ShaderProgram();

		CompileResult compile(ShaderSources &&sources);

		bool enable();
		void disable();

		void setParameter(const std::string &parameterName, ShaderParameter &&parameter);
		void addDefinition(const ShaderDefinition &definition);
		void removeDefinition(const ShaderDefinition &definition);
		void clearDefinitions();
		[[nodiscard]] bool hasDefinition(const ShaderDefinition &definition) const;

	private:
		struct ParameterInfo
		{
			int location;
		};

		int m_id { kInvalidId };
		bool m_needRebuild { true };
		bool m_isInUse { false };
		std::map<std::string, ParameterInfo> m_parameters {};
		std::set<ShaderDefinition> m_definitions {};
		std::map<ShaderSourceCodeType, std::string> m_sources {};
	};
}