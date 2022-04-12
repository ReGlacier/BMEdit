#pragma once

#include <GameLib/Type.h>
#include <GameLib/ValueView.h>
#include <variant>


namespace gamelib
{
	/**
	 * @class TypeComplex
	 * @brief This class represents
	 */
	class TypeComplex final : public Type
	{
		friend class TypeRegistry;
	public:
		TypeComplex(std::string typeName, std::vector<ValueView> &&instructionViews, Type *parent, bool allowUnexposedInstructions);
		TypeComplex(std::string typeName, std::vector<ValueView> &&instructionViews, std::string parentType, bool allowUnexposedInstructions);

		[[nodiscard]] const std::vector<ValueView> &getInstructionViews() const;
		[[nodiscard]] const Type *getParent() const;
		[[nodiscard]] bool areUnexposedInstructionsAllowed() const;

		[[nodiscard]] Span<prp::PRPInstruction> verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const override;
		[[nodiscard]] Type::DataMappingResult map(const Span<prp::PRPInstruction> &instructions) const override;

	private:
		std::vector<ValueView> m_instructionViews {};
		TypeReference m_parent {};
		bool m_allowUnexposedInstructions { false };
	};
}