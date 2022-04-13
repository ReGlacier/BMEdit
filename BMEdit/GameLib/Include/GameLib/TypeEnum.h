#pragma once

#include <GameLib/Type.h>

#include <vector>


namespace gamelib
{
	class TypeEnum final : public Type
	{
	public:
		struct Entry
		{
			std::string name;
			uint32_t value;
		};

		using Entries = std::vector<Entry>;

		TypeEnum(std::string typeName, Entries possibleValues);

		[[nodiscard]] Span<prp::PRPInstruction> verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;

		[[nodiscard]] const Entries &getPossibleValues() const;
	private:
		Entries m_possibleValues;
	};
}