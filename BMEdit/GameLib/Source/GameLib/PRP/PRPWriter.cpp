#include <GameLib/PRP/PRPWriter.h>
#include <GameLib/PRP/PRPHeader.h>
#include <GameLib/PRP/PRPByteCode.h>
#include <ZBinaryWriter.hpp>
#include <type_traits>


namespace gamelib::prp
{
	void buildTokenTableAndCacheObjectsCount(const PRPZDefines &definitions, const std::vector<PRPInstruction> &instructions, PRPTokenTable &tokenTable, int &objectsCount, uint32_t &dataOffset)
	{
		// Save string references from ZDefines
		for (const auto &def: definitions.getDefinitions())
		{
			const auto defType = def.getType();

			dataOffset += tokenTable.addToken(def.getName()) * (def.getName().length() + 1);

			if ((defType >= PRPDefinitionType::StringRef_1 && defType <= PRPDefinitionType::StringRef_3) || defType == PRPDefinitionType::StringRefTab)
			{
				std::visit(
					[&tokenTable, &dataOffset](auto&& value)
					{
						using T = std::decay_t<decltype(value)>;

						if constexpr (std::is_same_v<T, StringRef>)
						{
							dataOffset += tokenTable.addToken(value) * (value.length() + 1);
						}
						else if constexpr (std::is_same_v<T, StringRefTab>)
						{
							for (const auto& tok: value)
							{
								dataOffset += tokenTable.addToken(tok) * (tok.length() + 1);
							}
						}
					},
					def.getValue());
			}
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

			if (
				opCode == PRPOpCode::String || opCode == PRPOpCode::NamedString ||
				opCode == PRPOpCode::StringOrArray_E || opCode == PRPOpCode::StringOrArray_8E)
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