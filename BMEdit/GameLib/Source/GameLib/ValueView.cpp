#include <GameLib/ValueView.h>
#include <GameLib/Type.h>


namespace gamelib
{
	ValueView::ValueView() = default;

	ValueView::ValueView(std::string viewName, const Type *type, const Type *ownerType)
		: m_type(type), m_ownerType(ownerType), m_name(std::move(viewName))
	{
	}

	ValueView::ValueView(std::string viewName, std::string typeName, const Type *ownerType)
		: m_type(std::move(typeName)), m_ownerType(ownerType), m_name(std::move(viewName))
	{
	}

	ValueView::ValueView(std::string viewName, prp::PRPOpCode typeOpCode, const Type *ownerType)
		: m_type(typeOpCode), m_ownerType(ownerType), m_name(std::move(viewName))
	{
	}

	const Type *ValueView::getType() const
	{
		if (auto typePtr = std::get_if<const Type *>(&m_type); typePtr != nullptr)
		{
			return *typePtr;
		}

		return nullptr;
	}

	prp::PRPOpCode ValueView::getTrivialType() const
	{
		if (auto typePtr = std::get_if<prp::PRPOpCode>(&m_type); typePtr != nullptr)
		{
			return *typePtr;
		}

		return prp::PRPOpCode::ERR_UNKNOWN;
	}

	bool ValueView::isTrivialType() const
	{
		if (auto typePtr = std::get_if<prp::PRPOpCode>(&m_type); typePtr != nullptr)
		{
			return true;
		}

		return false;
	}

	const Type * ValueView::getOwnerType() const
	{
		return m_ownerType;
	}

	const std::string &ValueView::getName() const
	{
		return m_name;
	}
}