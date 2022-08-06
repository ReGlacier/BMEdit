#pragma once

#include <GameLib/Type.h>
#include <GameLib/PRP/PRPOpCode.h>
#include <variant>


namespace gamelib
{
	class TypeAlias final : public Type
	{
		friend class TypeRegistry;

	public:
		TypeAlias(std::string name, std::string resultType);
		TypeAlias(std::string name, prp::PRPOpCode resultType);

		[[nodiscard]] VerificationResult verify(const Span<prp::PRPInstruction>& instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;
	private:
		TypeReference m_resultTypeInfo;
	};
}