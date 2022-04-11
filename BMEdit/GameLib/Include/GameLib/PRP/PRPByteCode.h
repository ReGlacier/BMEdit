#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/Span.h>
#include <GameLib/PRP/PRPHeader.h>
#include <GameLib/PRP/PRPTokenTable.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/PRP/PRPByteCodeContext.h>


namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace gamelib::prp
{
	class PRPByteCode
	{
	public:
		PRPByteCode() = default;

		bool parse(const uint8_t *data, int64_t size, const PRPHeader *header, const PRPTokenTable *tokenTable);

		[[nodiscard]] const std::vector<PRPInstruction> &getInstructions() const;

		static void serialize(
			const std::vector<PRPInstruction> &instructions,
			const PRPHeader *header,
			const PRPTokenTable *tokenTable,
			ZBio::ZBinaryWriter::BinaryWriter *binaryWriter);

	private:
		void prepareOpCode(PRPByteCodeContext &context, const PRPHeader *header, const PRPTokenTable *tokenTable);

	private:
		Span<uint8_t> m_buffer;
		std::vector<PRPInstruction> m_instructions;
	};
}