#pragma once

#include <GameLib/PRM/PRMEntries.h>


namespace gamelib
{
	/**
	 * @note It's not full implementation of reader (it's doing partial read, but enough to visualize level geometry in editor)
	 * @todo Need to complete reverse engineering of this stuff and make writer
	 */
	class PRMReader
	{
	public:
		PRMReader();

		bool parse(const std::uint8_t* pBuffer, std::size_t iBufferSize);

		prm::PrmFile&& takePrimitives();

	private:
		prm::PrmFile m_file {};
	};
}