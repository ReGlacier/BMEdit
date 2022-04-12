#include <GameLib/TypeAlias.h>
#include <stdexcept>


namespace gamelib
{
	TypeAlias::TypeAlias(std::string name, std::string resultType)
		: Type(TypeKind::ALIAS, std::move(name)), m_resultTypeInfo(std::move(resultType))
	{
	}

	TypeAlias::TypeAlias(std::string name, prp::PRPOpCode resultType)
		: Type(TypeKind::ALIAS, std::move(name)), m_resultTypeInfo(resultType)
	{
	}

	Span<prp::PRPInstruction> TypeAlias::verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const
	{
		// Type is alias to another type
		if (auto typePtr = std::get_if<const Type *>(&m_resultTypeInfo); typePtr != nullptr) {
			return (*typePtr)->verifyInstructionSet(instructions);
		}

		// Type is alias to op-code
		if (auto resultOpCode = std::get_if<prp::PRPOpCode>(&m_resultTypeInfo); resultOpCode != nullptr) {
			if (instructions.size < 1) {
				return {};
			}

			const auto &requiredOpCode = *resultOpCode;
			if (requiredOpCode != instructions[0].getOpCode()) {
				return {};
			}

			return instructions.slice(1, instructions.size - 1);
		}

		// Alias not inited
		throw std::runtime_error("TypeAlias::verifyInstructionSet() failed. Alias not inited yet!");
	}

	Type::DataMappingResult TypeAlias::map(const Span<prp::PRPInstruction> &instructions) const
	{
		// Re-route request to type
		if (auto typePtr = std::get_if<const Type *>(&m_resultTypeInfo); typePtr != nullptr) {
			return (*typePtr)->map(instructions);
		}

		// Take instruction
		if (auto typeOpCodeValue = std::get_if<prp::PRPOpCode>(&m_resultTypeInfo); typeOpCodeValue != nullptr) {
			if (instructions.size < 1) {
				return Type::DataMappingResult();
			}

			const auto &typeOpCode = *typeOpCodeValue;
			if (typeOpCode != instructions[0].getOpCode()) {
				return Type::DataMappingResult();
			}

			return Type::DataMappingResult(Value(this, { instructions[0] }), instructions.slice(1, instructions.size - 1));
		}

		// Not inited yet
		throw std::runtime_error("TypeAlias::map() failed. Alias not inited yet!");
	}
}