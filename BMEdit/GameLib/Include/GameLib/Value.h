#pragma once

#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/ValueView.h>
#include <GameLib/Span.h>

#include <optional>
#include <vector>


namespace gamelib
{
	class Type;

	/**
	 * @struct ValueEntry
	 * @brief Basic representation of value entry. Each value could be represented via multiple values and there are must be combined into group of entries
	 * 		  Each entry contains name, reference to bunch of instructions (pos begin, size) and list of applied views
	 */
	struct ValueEntry
	{
		std::string name;
		struct {
			int64_t iOffset;
			int64_t iSize;

			[[nodiscard]] size_t offset() const { return static_cast<size_t>(iOffset); }
			[[nodiscard]] size_t size()   const { return static_cast<size_t>(iSize); }
		} instructions;
		std::vector<ValueView> views;
	};

	/**
	 * @class Value
	 * @brief The base representation of abstract definition in PRP.
	 *        Also, this class known as map between group of instructions and their view.
	 *        But generic value could contains a multiple views.
	 */
	class Value
	{
	public:
		Value();
		Value(const Type *type, std::vector<prp::PRPInstruction> data);
		Value(const Type *type, std::vector<prp::PRPInstruction> data, std::vector<ValueView> views);

		/**
		 * Store a single value without mapping (for trivial stuff)
		 * @param another
		 * @return
		 */
		Value& operator+=(const Value& another);

		/**
		 * Store a complex value (bunch of instructions) with mapping to virtual name
		 * @param another
		 * @return
		 */
		Value& operator+=(const std::pair<std::string, Value>& another);

		[[nodiscard]] const Type* getType() const;
		[[nodiscard]] const std::vector<prp::PRPInstruction>& getInstructions() const;
		[[nodiscard]] Span<ValueEntry> getEntries() const;
	private:
		const Type *m_type {nullptr}; // type
		std::vector<prp::PRPInstruction> m_data; // instructions
		std::vector<ValueEntry> m_entries; // entries
		std::vector<ValueView> m_views; // bruh
	};
}