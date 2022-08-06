#pragma once

#include <string>
#include <variant>

#include <GameLib/PRP/PRPOpCode.h>


namespace gamelib
{
	class Type;

	class ValueView
	{
		friend class TypeRegistry;

	public:
		ValueView();
		ValueView(std::string viewName, const Type *type, const Type *ownerType);
		ValueView(std::string viewName, std::string typeName, const Type *ownerType);
		ValueView(std::string viewName, prp::PRPOpCode typeOpCode, const Type *ownerType);

		[[nodiscard]] const Type *getType() const;
		[[nodiscard]] bool isTrivialType() const;
		[[nodiscard]] prp::PRPOpCode getTrivialType() const;
		[[nodiscard]] const Type *getOwnerType() const;
		[[nodiscard]] const std::string &getName() const;

		[[nodiscard]] bool operator==(const ValueView& other) const noexcept;
		[[nodiscard]] bool operator!=(const ValueView& other) const noexcept;

	private:
		std::variant<std::string, const Type *, prp::PRPOpCode> m_type;
		const Type *m_ownerType { nullptr };
		std::string m_name {};
	};
}