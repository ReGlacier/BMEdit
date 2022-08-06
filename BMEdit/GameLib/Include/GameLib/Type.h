#pragma once

#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/TypeKind.h>
#include <GameLib/Value.h>
#include <GameLib/Span.h>
#include <string>
#include <vector>
#include <optional>
#include <variant>


namespace gamelib
{
	class Type
	{
	public:
		Type() = default;
		Type(TypeKind kind, std::string name);
		virtual ~Type() noexcept;

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] TypeKind getKind() const;

		using VerificationResult = std::pair<bool, Span<prp::PRPInstruction>>; // [0] - result, [1] - slice of unused instructions
		/**
		 * @fn verify
		 * @param instructions - span of vector of instructions which should represent valid data
		 * @return [true, span] - when verification passed, [false, nullptr] - when verification failed
		 * @example:
		 * 		If we have enum with possible values "A1", "A2", "A3" and instructionSet is equal to { OpCode: String, StrValue: "ZERO" }
		 * 		this data will be invalid because enumerator require to have value which enumerated in internal set but "ZERO" value not enumerated in it.
		 * @note TypeAlias will redirect this check to target class
		 */
		[[nodiscard]] virtual VerificationResult verify(const Span<prp::PRPInstruction>& instructions) const;

		using DataMappingResult = std::pair<std::optional<Value>, Span<prp::PRPInstruction>>; // [0] - mapped data, [1] - new slice of instructions
		/**
		 * @fn map
		 * @param instructionSet - list of instructions
		 * @param startInstructionIndex - start index
		 * @return pair between result value and index of next instruction. Function failed if value is not set or index less than zero.
		 */
		[[nodiscard]] virtual DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const;

	private:
		std::string m_name {};
		TypeKind m_kind { TypeKind::NONE };
	};

	using TypeReference = std::variant<
		std::string,
		const Type *,
		prp::PRPOpCode
	>;
}