#include <gtest/gtest.h>

#include <GameLib/PRP/PRP.h>
#include <GameLib/PRP/PRPHeader.h>
#include <GameLib/PRP/PRPReader.h>
#include <GameLib/PRP/PRPWriter.h>
#include <GameLib/PRP/PRPTokenTable.h>

// Usage
using gamelib::prp::PRPReader;
using gamelib::prp::PRPWriter;
using gamelib::prp::PRPHeader;
using gamelib::prp::PRPTokenTable;
using gamelib::prp::PRPByteCode;
using gamelib::prp::PRPOpCode;

// Our tests
TEST(PRP, Decompiler_SimpleInstructionDecoder)
{
	PRPHeader header(1u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("ROOT");
	tokenTable.addToken("Hitman");

	PRPByteCode byteCode;

	// Here is a trivial instruction
	const uint8_t kBuffer[] = {
	    (uint8_t)(PRPOpCode::String),
	    0x01, 0x00, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::EndOfStream)
	};

	ASSERT_TRUE(byteCode.parse(&kBuffer[0], sizeof(kBuffer), &header, &tokenTable)) << "Failed to decompile byte code";
	ASSERT_EQ(byteCode.getInstructions().size(), 2) << "Wrong count of instructions";

	ASSERT_EQ(byteCode.getInstructions()[0].getOpCode(), PRPOpCode::String);
	ASSERT_EQ(byteCode.getInstructions()[0].getOperand().str, "Hitman");

	ASSERT_EQ(byteCode.getInstructions()[1].getOpCode(), PRPOpCode::EndOfStream);
}