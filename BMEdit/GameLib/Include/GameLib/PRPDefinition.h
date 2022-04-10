#pragma once

#include <string>
#include <variant>
#include <vector>
#include <GameLib/PRPDefinitionType.h>


namespace gamelib::prp
{
	using ArrayI32 = std::vector<int32_t>;
	using ArrayF32 = std::vector<float>;
	using StringRef = std::string;
	using StringRefTab = std::vector<std::string>;

	using PRPDefinitionValue = std::variant<
		ArrayI32,
		ArrayF32,
		StringRef,
		StringRefTab>;

	class PRPDefinition
	{
	public:
		PRPDefinition() = default;
		PRPDefinition(std::string name, PRPDefinitionType definitionType, PRPDefinitionValue value);

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] PRPDefinitionType getType() const;
		[[nodiscard]] const PRPDefinitionValue &getValue() const;

		void setValue(const PRPDefinitionValue &value);
		void setValue(const PRPDefinitionValue &value, PRPDefinitionType type);

		explicit operator bool() const noexcept;

	private:
		static PRPDefinitionType valueToType(const PRPDefinitionValue& value);
		void updateIsSet();

	private:
		std::string m_name{};
		PRPDefinitionType m_type{PRPDefinitionType::ERR_UNKNOWN};
		PRPDefinitionValue m_value{};
		bool m_isSet{false};
	};
}