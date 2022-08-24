#include <GameLib/PRP/PRPDefinition.h>
#include <ZBinaryWriter.hpp>

#include <utility>
#include <type_traits>


namespace gamelib::prp
{
	PRPDefinition::PRPDefinition(std::string name,
	                             PRPDefinitionType definitionType,
	                             PRPDefinitionValue value)
		: m_name(std::move(name)), m_type(definitionType), m_value(std::move(value))
	{
		updateIsSet();
	}

	const std::string &PRPDefinition::getName() const
	{
		return m_name;
	}

	PRPDefinitionType PRPDefinition::getType() const
	{
		return m_type;
	}

	const PRPDefinitionValue &PRPDefinition::getValue() const
	{
		return m_value;
	}

	void PRPDefinition::setName(const std::string &newName)
	{
		m_name = newName;
	}

	void PRPDefinition::setValue(const PRPDefinitionValue &value)
	{
		m_value = value;

		const auto newType = valueToType(m_value);
		const bool newTypeIsStringRef = (newType >= PRPDefinitionType::StringRef_1 && newType <= PRPDefinitionType::StringRef_3);
		const bool oldTypeIsStringRef = (m_type >= PRPDefinitionType::StringRef_1 && m_type <= PRPDefinitionType::StringRef_3);

		if (newTypeIsStringRef ^ oldTypeIsStringRef) {
			m_type = newType;
		}

		updateIsSet();
	}

	void PRPDefinition::setValue(const PRPDefinitionValue &value, PRPDefinitionType type)
	{
		m_value = value;
		m_type = type;

		updateIsSet();
	}

	bool PRPDefinition::operator==(const gamelib::prp::PRPDefinition &other) const
	{
		if (m_isSet == other.m_isSet && !m_isSet) return true;

		if (m_isSet != other.m_isSet) return false;
		if (m_name != other.m_name) return false;
		if (m_type != other.m_type) return false;
		if (m_value != other.m_value) return false;

		return true;
	}

	bool PRPDefinition::operator!=(const gamelib::prp::PRPDefinition &other) const
	{
		return !operator==(other);
	}

	PRPDefinition::operator bool() const noexcept
	{
		return m_isSet;
	}

	PRPDefinitionType PRPDefinition::valueToType(const PRPDefinitionValue &value)
	{
		PRPDefinitionType result = PRPDefinitionType::ERR_UNKNOWN;

		std::visit(
			[&result](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, ArrayI32>) {
					result = PRPDefinitionType::Array_Int32;
				}
				else if constexpr (std::is_same_v<T, ArrayF32>) {
					result = PRPDefinitionType::Array_Float32;
				}
				else if constexpr (std::is_same_v<T, StringRef>) {
					result = PRPDefinitionType::StringRef_1;
					// Unable to recognize other types
				}
				else if constexpr (std::is_same_v<T, StringRefTab>) {
					result = PRPDefinitionType::StringRefTab;
				}
			},
			value);

		return result;
	}

	void PRPDefinition::updateIsSet()
	{
		m_isSet = valueToType(m_value) != PRPDefinitionType::ERR_UNKNOWN;
	}
}