#pragma once

#include <GameLib/Type.h>
#include <GameLib/ValueView.h>
#include <GameLib/PRP/PRPOpCode.h>


namespace gamelib
{
	class TypeArray final : public Type
	{
		friend class TypeRegistry;
	public:
		TypeArray(std::string typeName, prp::PRPOpCode entryType, uint32_t requiredCapacity);
		TypeArray(std::string typeName, prp::PRPOpCode entryType, uint32_t requiredCapacity, std::vector<ValueView> &&valueViews);

		[[nodiscard]] prp::PRPOpCode getTypeOfEntry() const;
		[[nodiscard]] uint32_t getRequiredCapacity() const;
		[[nodiscard]] const std::vector<ValueView> &getValueViews() const;

		[[nodiscard]] VerificationResult verify(const Span<prp::PRPInstruction>& instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;
	private:
		prp::PRPOpCode m_entryType { prp::PRPOpCode::ERR_UNKNOWN };
		uint32_t m_requiredCapacity { 0u };
		std::vector<ValueView> m_valueViews;
	};
}