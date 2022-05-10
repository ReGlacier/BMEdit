#include <GameLib/GeomBasedTypeInfo.h>


namespace gamelib
{
	GeomBasedTypeInfo::GeomBasedTypeInfo() = default;

	GeomBasedTypeInfo::GeomBasedTypeInfo(uint32_t typeId, uint32_t mask, uint32_t id)
		: m_typeId(typeId), m_mask(mask), m_id(id)
	{
	}

	uint32_t GeomBasedTypeInfo::getTypeId() const
	{
		return m_typeId;
	}

	uint32_t GeomBasedTypeInfo::getMask() const
	{
		return m_mask;
	}

	uint32_t GeomBasedTypeInfo::getId() const
	{
		return m_id;
	}
}