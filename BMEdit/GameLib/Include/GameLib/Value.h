#pragma once

#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/ValueView.h>
#include <vector>
#include <optional>


namespace gamelib
{
	class Type;

	class Value
	{
	public:
		Value();
		Value(const Type *type, std::vector<prp::PRPInstruction> data);
		Value(const Type *type, std::vector<prp::PRPInstruction> data, std::vector<ValueView> views);

		Value& operator+=(const Value& another);

		[[nodiscard]] const Type *getType() const;
		[[nodiscard]] const std::vector<prp::PRPInstruction> &getInstructions() const;
		[[nodiscard]] int getInstructionsCount() const;
		[[nodiscard]] const std::vector<ValueView> &getViews() const;
		[[nodiscard]] const ValueView& getView(int instructionIndex) const;
		[[nodiscard]] bool hasView(int instructionIndex) const;

		bool setInstruction(int instructionIndex, const prp::PRPInstruction& instruction);
		bool getInstruction(int instructionIndex, prp::PRPInstruction &instruction);
	private:
		const Type *m_type {nullptr};
		std::vector<prp::PRPInstruction> m_data;
		std::vector<ValueView> m_views;
	};
}