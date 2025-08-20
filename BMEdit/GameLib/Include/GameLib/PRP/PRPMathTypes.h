#pragma once

#include <GameLib/PRP/PRPObjectExtractor.h>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>


namespace gamelib
{
	template<>
	struct TObjectExtractor<bool>
	{
		static bool extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				return instructions[0].getOperand().get<bool>();
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<std::int8_t>
	{
		static std::int8_t extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				return instructions[0].getOperand().get<std::int8_t>();
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<std::int16_t>
	{
		static std::int16_t extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				return instructions[0].getOperand().get<std::int16_t>();
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<std::int32_t>
	{
		static std::int32_t extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				return instructions[0].getOperand().get<std::int32_t>();
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<float>
	{
		static float extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				return instructions[0].getOperand().get<float>();
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<std::string>
	{
		static std::string extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 1)
			{
				const std::string& v = instructions[0].getOperand().get<const std::string&>();
				return v;
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<glm::vec2>
	{
		static glm::vec2 extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 4) // [0] begin array, [-1] end array
			{
				return {
				    instructions[1].getOperand().get<float>(),
				    instructions[2].getOperand().get<float>()
				};
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<glm::vec3>
	{
		static glm::vec3 extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 5) // [0] begin array, [-1] end array
			{
				return {
				    instructions[1].getOperand().get<float>(),
				    instructions[2].getOperand().get<float>(),
				    instructions[3].getOperand().get<float>()
				};
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<glm::vec4>
	{
		static glm::vec4 extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 6) // [0] begin array, [-1] end array
			{
				return {
				    instructions[1].getOperand().get<float>(),
				    instructions[2].getOperand().get<float>(),
				    instructions[3].getOperand().get<float>(),
				    instructions[4].getOperand().get<float>()
				};
			}

			assert(false && "Bad object");
			return {};
		}
	};

	template<>
	struct TObjectExtractor<glm::mat3>
	{
		static glm::mat3 extract(const Span<prp::PRPInstruction>& instructions)
		{
			if (instructions.size() == 11) // [0] begin array, [-1] end array
			{
				return {
				    instructions[1].getOperand().get<float>(),
				    instructions[2].getOperand().get<float>(),
				    instructions[3].getOperand().get<float>(),
				    instructions[4].getOperand().get<float>(),
				    instructions[5].getOperand().get<float>(),
				    instructions[6].getOperand().get<float>(),
				    instructions[7].getOperand().get<float>(),
				    instructions[8].getOperand().get<float>(),
				    instructions[9].getOperand().get<float>()
				};
			}

			assert(false && "Bad object");
			return {};
		}
	};
}