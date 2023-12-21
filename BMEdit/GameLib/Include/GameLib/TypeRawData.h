#pragma once

#include <GameLib/Type.h>


namespace gamelib
{
	class TypeRawData final : public Type
	{
	public:
		explicit TypeRawData(std::string typeName);

		[[nodiscard]] VerificationResult verify(const Span<prp::PRPInstruction>& instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;
		[[nodiscard]] Value makeDefaultPropertiesPack() const override;
	};
}