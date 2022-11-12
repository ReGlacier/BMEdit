#pragma once

#include <string>
#include <variant>


namespace gamelib
{
	class Type;

	class TypeHint
	{
	public:
		TypeHint();
		explicit TypeHint(std::string typeName);
		explicit TypeHint(Type* typeInstance);

		TypeHint& operator=(Type* typeInstance);

		[[nodiscard]] bool isSet() const;
		[[nodiscard]] bool isResolved() const;
		[[nodiscard]] std::string getTypeName() const;
		[[nodiscard]] Type* getType() const;

	private:
		struct NullHint {};

		std::variant<NullHint, std::string, Type*> m_hint {};
	};
}