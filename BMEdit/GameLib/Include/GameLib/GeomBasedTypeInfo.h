#pragma once

#include <cstdint>


namespace gamelib
{
	class GeomBasedTypeInfo
	{
	public:
		GeomBasedTypeInfo();
		GeomBasedTypeInfo(uint32_t typeId, uint32_t mask, uint32_t id);

		[[nodiscard]] uint32_t getTypeId() const;
		[[nodiscard]] uint32_t getId() const;
		[[nodiscard]] uint32_t getMask() const;

	private:
		uint32_t m_typeId;
		uint32_t m_mask;
		uint32_t m_id;
	};
}