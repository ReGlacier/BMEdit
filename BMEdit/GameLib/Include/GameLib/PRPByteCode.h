#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/Span.h>
#include <GameLib/PRPHeader.h>
#include <GameLib/PRPTokenTable.h>
#include <GameLib/PRPInstruction.h>
#include <GameLib/PRPByteCodeContext.h>


namespace gamelib::prp
{
	class PRPByteCode
	{
	public:
		PRPByteCode() = default;

		bool parse(const uint8_t *data, int64_t size, const PRPHeader *header, const PRPTokenTable *tokenTable);

		[[nodiscard]] const std::vector<PRPInstruction> &getInstructions() const;

	private:
		void prepareOpCode(PRPByteCodeContext& context, const PRPHeader *header, const PRPTokenTable *tokenTable);

	private:
		Span<uint8_t> m_buffer;
		std::vector<PRPInstruction> m_instructions;
	};
}