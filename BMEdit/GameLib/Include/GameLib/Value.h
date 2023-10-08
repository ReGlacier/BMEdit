#pragma once

#include <GameLib/PRP/PRPObjectExtractor.h>
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

		[[nodiscard]] bool operator==(const ValueEntry &other) const
		{
			if (this == &other) return true;

			return
			    name == other.name &&
			    views == other.views &&
			    instructions.iOffset == other.instructions.iOffset &&
			    instructions.iSize == other.instructions.iSize;
		}

		[[nodiscard]] bool operator!=(const ValueEntry &other) const
		{
			return !operator==(other);
		}
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

		/**
		 * Extract span of instructions which represents requested property
		 * @param propertyName - name of the property
		 * @return span of instructions
		 * @note This function may throw an exception if requested propertyName not found. You should check your property via hasProperty before ask operator[]
		 */
		Span<prp::PRPInstruction> operator[](const char* propertyName);

		[[nodiscard]] bool operator==(const Value &other) const;
		[[nodiscard]] bool operator!=(const Value &other) const;

		[[nodiscard]] const Type* getType() const;
		[[nodiscard]] const std::vector<prp::PRPInstruction>& getInstructions() const;
		[[nodiscard]] std::vector<prp::PRPInstruction>& getInstructions();
		[[nodiscard]] Span<ValueEntry> getEntries() const;

		void updateContainer(int entryIndex, const std::vector<prp::PRPInstruction>& newDecl);

		bool hasProperty(const char* propertyName) const;

		// Extractor
		template <typename T>
		T getObject(const std::string& objectName, T def = T()) const requires (HasSpecializationTObjectExtractor<T>)
		{
			for (const auto& ent : m_entries)
			{
				if (ent.name == objectName)
				{
					return TObjectExtractor<T>::extract(Span(m_data).slice(ent.instructions));
				}
			}

			return def;
		}

	private:
		const Type *m_type {nullptr}; // type
		std::vector<prp::PRPInstruction> m_data; // instructions
		std::vector<ValueEntry> m_entries; // entries
		std::vector<ValueView> m_views; // bruh
	};
}