#include <GameLib/TypeHint.h>
#include <GameLib/Type.h>


namespace gamelib
{
	TypeHint::TypeHint() = default;

	TypeHint::TypeHint(std::string typeName)
	{
		if (!typeName.empty())
		{
			m_hint = std::move(typeName);
		}
	}

	TypeHint::TypeHint(gamelib::Type *typeInstance)
	{
		if (typeInstance)
		{
			m_hint = typeInstance;
		}
	}

	TypeHint &TypeHint::operator=(gamelib::Type *typeInstance)
	{
		if (!typeInstance)
		{
			m_hint = NullHint {};
		}
		else
		{
			m_hint = typeInstance;
		}

		return *this;
	}

	bool TypeHint::isSet() const
	{
		return std::get_if<NullHint>(&m_hint) == nullptr;
	}

	bool TypeHint::isResolved() const
	{
		return std::get_if<Type*>(&m_hint) != nullptr;
	}

	std::string TypeHint::getTypeName() const
	{
		if (auto tp = std::get_if<std::string>(&m_hint))
		{
			return *tp;
		}

		return {};
	}

	Type *TypeHint::getType() const
	{
		if (auto tp = std::get_if<Type*>(&m_hint))
		{
			return *tp;
		}

		return nullptr;
	}
}