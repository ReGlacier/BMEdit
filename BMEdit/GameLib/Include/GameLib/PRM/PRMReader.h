#pragma once

#include <GameLib/PRM/PRMHeader.h>
#include <GameLib/PRM/PRMPrimitive.h>
#include <GameLib/Span.h>
#include <cstdint>
#include <vector>


namespace gamelib::prm
{
	class PRMReader
	{
	public:
		PRMReader() = default;

		bool read(Span<uint8_t> buffer);

		[[nodiscard]] const PRMHeader &getHeader() const;
		[[nodiscard]] const std::vector<PRMPrimitive> &getPrimitives() const;

	private:
		PRMHeader m_header;
		std::vector<PRMPrimitive> m_primitives;
	};
}