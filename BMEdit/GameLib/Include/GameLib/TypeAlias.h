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
		[[nodiscard]] Value makeDefaultPropertiesPack() const override;

		[[nodiscard]] const Type* getFinalType() const;
		[[nodiscard]] prp::PRPOpCode getFinalOpCode() const;

	public: // Additional & tooling
		[[nodiscard]] bool hasToolHint() const;
		[[nodiscard]] const std::string& getToolHint() const;
		void setToolHint(const std::string& toolHint);

	private:
		TypeReference m_resultTypeInfo;
		std::string m_toolHint {};
	};
}