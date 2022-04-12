#pragma once

#include <GameLib/Type.h>
#include <string>
#include <map>


namespace gamelib
{
	class TypeBitfield final : public Type
	{
	public:
		using PossibleOptions = std::map<std::string, uint32_t>;

		TypeBitfield(std::string typeName, TypeBitfield::PossibleOptions &&possibleOptions);

		[[nodiscard]] const TypeBitfield::PossibleOptions &getPossibleOptions() const;

		[[nodiscard]] Span<prp::PRPInstruction> verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;
	private:
		TypeBitfield::PossibleOptions m_possibleOptions;
	};
}