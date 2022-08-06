#pragma once

#include <GameLib/Type.h>


namespace gamelib
{
	/**
	 * @class TypeContainer
	 * @brief Unlike TypeArray this type can contains dynamic count of objects of different type
	 */
	class TypeContainer final : public Type
	{
	public:
		explicit TypeContainer(std::string typeName);

		[[nodiscard]] VerificationResult verify(const Span<prp::PRPInstruction>& instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;
	};
}