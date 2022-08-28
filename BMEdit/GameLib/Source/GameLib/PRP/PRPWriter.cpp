#include <GameLib/PRP/PRPWriter.h>
#include <GameLib/PRP/PRPHeader.h>
#include <GameLib/PRP/PRPByteCode.h>
#include <ZBinaryWriter.hpp>
#include <type_traits>


namespace gamelib::prp
{
	struct ZDefineStringVisitor
	{
		uint32_t &dataOffset;
		PRPTokenTable &tokenTable;

		ZDefineStringVisitor(uint32_t &offset, PRPTokenTable &tokTable) : dataOffset(offset), tokenTable(tokTable) {}

		void operator()(const StringRef &stringRef)
		{
			if (tokenTable.addToken(stringRef))
			{
				dataOffset += stringRef.length() + 1;
			}
		}

		void operator()(const StringRefTab &stringRefTab)
		{
			for (const auto& token: stringRefTab)
			{
				operator()(token);
			}
		}

		void operator()(const ArrayI32&) {} // Do nothing
		void operator()(const ArrayF32&) {} // Do nothing
	};

	void buildTokenTableAndCacheObjectsCount(const PRPZDefines &definitions, const std::vector<PRPInstruction> &instructions, PRPTokenTable &tokenTable, int &objectsCount, uint32_t &dataOffset)
	{
		ZDefineStringVisitor visitor(dataOffset, tokenTable);

		// Save string references from ZDefines
		for (const auto &def: definitions.getDefinitions())
		{
			if (tokenTable.addToken(def.getName()))
			{
				dataOffset += (def.getName().length() + 1);
			}

			std::visit(visitor, def.getValue());
		}

		// Save string references from instructions
		for (const auto& instruction: instructions)
		{
			const auto opCode = instruction.getOpCode();
			objectsCount += 1 * (opCode == PRPOpCode::BeginObject || opCode == PRPOpCode::BeginNamedObject);

			if (opCode == PRPOpCode::StringArray)
			{
				for (const auto &str: instruction.getOperand().stringArray)
				{
					dataOffset += tokenTable.addToken(str) * (str.length() + 1);
				}
			}

			if (opCode == PRPOpCode::String || opCode == PRPOpCode::NamedString)
			{
				const auto& tok = instruction.getOperand().str;
				dataOffset += tokenTable.addToken(tok) * (tok.length() + 1);
			}
			else if (opCode == PRPOpCode::StringOrArray_E || opCode == PRPOpCode::StringOrArray_8E)
			{
				const auto& tok = instruction.getOperand().str;
				dataOffset += tokenTable.addToken(tok) * (tok.length() + 1);
			}
		}
	}

	void PRPWriter::write(const PRPZDefines &definitions,
	                      const std::vector<PRPInstruction> &instructions,
	                      bool isRaw,
	                      std::vector<uint8_t> &outBuffer)
	{
		auto writerSink = std::make_unique<ZBio::ZBinaryWriter::BufferSink>();
		auto binaryWriter = ZBio::ZBinaryWriter::BinaryWriter(std::move(writerSink));

		// Build token table
		PRPTokenTable tokenTable {};
		int objectsCount { 0 };
		uint32_t dataOffset { 0x1F };

		buildTokenTableAndCacheObjectsCount(definitions, instructions, tokenTable, objectsCount, dataOffset);
		if (dataOffset > 0x1F)
		{
			dataOffset -= 0x1F;
		}
		else
		{
			assert(false); // Calculation error
		}

		// Create header
		PRPHeader header(tokenTable.getNonEmptyTokenCount(), isRaw, false, true);
		PRPHeader::serialize(header, dataOffset, &binaryWriter);

		// Write token table
		PRPTokenTable::serialize(tokenTable, &binaryWriter);

		// Write objects count
		binaryWriter.write<uint32_t, ZBio::Endianness::LE>(objectsCount);

		// Write zdefs
		PRPZDefines::serialize(definitions, &tokenTable, &binaryWriter);

		// Write instructions
		PRPByteCode::serialize(instructions, &header, &tokenTable, &binaryWriter);

		// Save result to buffer
		auto raw = binaryWriter.release().value();
		std::copy(raw.begin(), raw.end(), std::back_inserter(outBuffer));
	}
}