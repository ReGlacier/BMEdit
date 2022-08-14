#include <GameLib/PRP/PRPByteCode.h>
#include <GameLib/PRP/PRPStructureError.h>
#include <GameLib/PRP/PRPBadInstruction.h>
#include <GameLib/PRP/PRPOpCodeNotImplemented.h>
#include <GameLib/PRP/PRPBadStringReference.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>
#include <cassert>


namespace gamelib::prp
{
	namespace opc
	{
		struct OpCodeDescription {
			using LoadHandler = void(*)(const Span<uint8_t> &buffer, PRPByteCodeContext&, PRPOpCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &outInstructions);
			using SaveHandler = void(*)(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);

			PRPOpCode opCode { PRPOpCode::ERR_UNKNOWN };
			int operandSize { 0 };
			LoadHandler loadHandler { nullptr };
			SaveHandler saveHandler { nullptr };

			constexpr OpCodeDescription(PRPOpCode _opCode, int _operandSize, LoadHandler _loadHandler, SaveHandler _saveHandler = nullptr)
				: opCode(_opCode)
				, operandSize(_operandSize)
				, loadHandler(_loadHandler)
				, saveHandler(_saveHandler)
			{
			}

			void operator()(const Span<uint8_t>& buffer, PRPByteCodeContext& context, const PRPHeader *header, const PRPTokenTable *tokenTable, std::vector<PRPInstruction> &outInstructions) const
			{
				// Prepare operand stack
				constexpr int kMaxOperandSize = 16;

				uint8_t operandMemory[kMaxOperandSize] { 0 };
				uint8_t* operandDataPtr { nullptr };

				if (operandSize)
				{
					assert(operandSize <= kMaxOperandSize); // Not enough local size

					operandDataPtr = &operandMemory[0];

					// Extract N bytes from ctx and skip 'em
					std::memcpy(operandDataPtr, &buffer[context.getIndex()], operandSize);
					context += operandSize;
				}

				// Call 'load' handler
				assert(loadHandler != nullptr); // load handler is required!
				if (loadHandler)
				{
					loadHandler(buffer, context, opCode, header, tokenTable, operandDataPtr, outInstructions);
				}
			}

			void operator()(const PRPInstruction &instruction, const PRPHeader *header, const PRPTokenTable *tokenTable, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter) const
			{
				// Call saveHandler if it defined, otherwise is ok (no extra data or op-code is enough)
				if (saveHandler)
				{
					saveHandler(instruction, header, tokenTable, binaryWriter);
				}
			}
		};

		template <PRPByteCodeContext::ContextFlags cf>
		static void prepareBeginOfGenericContainer(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto capacity = *reinterpret_cast<const int32_t*>(operand);

			PRPOperandVal operandVal(capacity);
			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));

			context.setFlag(cf);
		}

		template <PRPByteCodeContext::ContextFlags cf>
		static void prepareEndOfGenericContainer(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &outInstructions)
		{
			outInstructions.emplace_back(PRPInstruction(opCode));
			context.unsetFlag(cf);
		}

		void prepareBeginObject(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareEndOfStream(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareBool(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareChar(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareInt8(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareInt16(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareInt32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareFloat32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareFloat64(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		PRPOperandVal exchangeString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand);
		void prepareString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareStringArray(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareRawData(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareSkipMark(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareEnum(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions);
		void prepareReference(const Span<uint8_t> &, PRPByteCodeContext &, PRPOpCode opCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &);

		void serializeTrivial(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);
		void serializeString(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);
		void serializeEnum(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);
		void serializeRawData(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);
		void serializeStringArray(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);
		void serializeArrayOrContainer(const PRPInstruction &, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *);

		// --- OPC handlers ---
		static constexpr opc::OpCodeDescription g_opCodeHandlers[] = {
			{ PRPOpCode::Array, 4, prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY>, serializeArrayOrContainer },
			{ PRPOpCode::NamedArray, 4, prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY>, serializeArrayOrContainer },
			{ PRPOpCode::EndArray, 0, prepareEndOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY> },
			{ PRPOpCode::BeginObject, 0, prepareBeginObject },
			{ PRPOpCode::BeginNamedObject, 0, prepareBeginObject },
			{ PRPOpCode::EndObject, 0, prepareEndOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_OBJECT> },
			{ PRPOpCode::Container, 4, prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_CONTAINER>, serializeArrayOrContainer },
			{ PRPOpCode::NamedContainer, 4, prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_CONTAINER>, serializeArrayOrContainer },
			{ PRPOpCode::EndOfStream, 0, prepareEndOfStream },
			{ PRPOpCode::Bool, 1, prepareBool, serializeTrivial },
			{ PRPOpCode::Char, 1, prepareChar, serializeTrivial },
			{ PRPOpCode::Int8, 1, prepareInt8, serializeTrivial },
			{ PRPOpCode::NamedInt8, 1, prepareInt8, serializeTrivial },
			{ PRPOpCode::Int16, 2, prepareInt16, serializeTrivial },
			{ PRPOpCode::NamedInt16, 2, prepareInt16, serializeTrivial },
			{ PRPOpCode::Int32, 4, prepareInt32, serializeTrivial },
			{ PRPOpCode::NamedInt32, 4, prepareInt32, serializeTrivial },
			{ PRPOpCode::Float32, 4, prepareFloat32, serializeTrivial },
			{ PRPOpCode::NamedFloat32, 4, prepareFloat32, serializeTrivial },
			{ PRPOpCode::Float64, 8, prepareFloat64, serializeTrivial },
			{ PRPOpCode::NamedFloat64, 8, prepareFloat64, serializeTrivial },
			{ PRPOpCode::Bitfield, 4, prepareInt32, serializeTrivial },
			{ PRPOpCode::NameBitfield, 4, prepareInt32, serializeTrivial },
			{ PRPOpCode::String, 4, prepareString, serializeString },
			{ PRPOpCode::NamedString, 4, prepareString, serializeString },
			{ PRPOpCode::StringArray, 4, prepareStringArray, serializeStringArray },
			{ PRPOpCode::RawData, 4, prepareRawData, serializeRawData },
			{ PRPOpCode::NamedRawData, 4, prepareRawData, serializeRawData },
			{ PRPOpCode::SkipMark, 0, prepareSkipMark },
			{ PRPOpCode::StringOrArray_E, 4, prepareEnum, serializeEnum },
			{ PRPOpCode::StringOrArray_8E, 4, prepareEnum, serializeEnum },
			{ PRPOpCode::Reference, 0, prepareReference }, // Not implemented
			{ PRPOpCode::NamedReference, 0, prepareReference }, // Not implemented
		};
	}

	bool PRPByteCode::parse(const uint8_t *data, int64_t size, const PRPHeader *header, const PRPTokenTable *tokenTable)
	{
		if (!data || !size  || !header || !tokenTable)
		{
			/// Maybe later we will support work without token table but not now
			return false;
		}

		m_buffer = Span { data, size };

		PRPByteCodeContext byteCodeContext(0); // Start from 0 instruction

		while (byteCodeContext.getIndex() < size) {
			prepareOpCode(byteCodeContext, header, tokenTable);
		}

		m_buffer.reset();
		return byteCodeContext.isEndOfStream();
	}

	const std::vector<PRPInstruction> &PRPByteCode::getInstructions() const
	{
		return m_instructions;
	}

	void PRPByteCode::prepareOpCode(PRPByteCodeContext &context,
	                                const PRPHeader *header,
	                                const PRPTokenTable *tokenTable)
	{
		auto opCode = FromBytes<PRPOpCode>()(m_buffer[context.getIndex()]);
		if (!OPCODE_VALID(opCode))
		{
			throw PRPBadInstruction("Invalid instruction", PRPRegionID::INSTRUCTIONS, context.getIndex());
		}
		++context; // Skip ready opcode

		for (const auto& handler: opc::g_opCodeHandlers)
		{
			if (handler.opCode != opCode)
			{
				continue;
			}

			handler(m_buffer, context, header, tokenTable, m_instructions);
			return;
		}

		throw PRPOpCodeNotImplemented("PRP OpCode " + to_string(opCode) + " not implemented!", PRPRegionID::INSTRUCTIONS, -1);
	}

	void PRPByteCode::serialize(const std::vector<PRPInstruction> &instructions,
	                            const PRPHeader *header,
	                            const PRPTokenTable *tokenTable,
	                            ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		for (const auto &instruction: instructions)
		{
			const auto opCode = instruction.getOpCode();

			// Find handler
			for (const auto &handler: opc::g_opCodeHandlers)
			{
				if (handler.opCode != opCode)
				{
					continue;
				}

				// Store op-code
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(instruction.getOpCode()));

				// Store data
				handler(instruction, header, tokenTable, binaryWriter);

				break;
			}
		}
	}
}

/// OPC IMPL HERE
namespace gamelib::prp::opc
{
	///-----------------
	/// DESERIALIZERS
	///-----------------
	void prepareBeginObject(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		outInstructions.emplace_back(PRPInstruction(opCode));
		context.setFlag(PRPByteCodeContext::ContextFlags::CF_READ_OBJECT);
	}

	void prepareEndOfStream(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		outInstructions.emplace_back(PRPInstruction(opCode));
		context.setFlag(PRPByteCodeContext::ContextFlags::CF_END_OF_STREAM);
	}

	void prepareBool(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const bool*>(operand);

		PRPOperandVal operandVal(a0);
		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareChar(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const char*>(operand);

		PRPOperandVal operandVal(a0);
		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareInt8(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const int8_t*>(operand);
		PRPOperandVal operandVal(a0);

		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareInt16(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const int16_t*>(operand);
		PRPOperandVal operandVal(a0);

		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareInt32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const int32_t*>(operand);
		PRPOperandVal operandVal(a0);

		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareFloat32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const float *>(operand);
		PRPOperandVal operandVal(a0);

		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	void prepareFloat64(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto a0 = *reinterpret_cast<const double *>(operand);
		PRPOperandVal operandVal(a0);

		outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
	}

	PRPOperandVal exchangeString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand)
	{
		const auto a0 = *reinterpret_cast<const int32_t *>(operand);

		if (header->isTokenTablePresented())
		{
			// a0 is token index
			if (!tokenTable->hasIndex(a0))
			{
				throw PRPBadStringReference("Bad string reference. Token #" + std::to_string(a0) + " not found", PRPRegionID::INSTRUCTIONS, context.getIndex());
			}

			return PRPOperandVal(tokenTable->tokenAt(a0));
		} else {
			// a0 is a length of string
			std::string str;

			if (a0) {
				str.resize(a0);
				std::memcpy(&str[0], &buffer[context.getIndex()], a0);
				context += a0; // skip a0 bytes
			}

			return PRPOperandVal(std::move(str));
		}
	}

	void prepareString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		auto opVal = exchangeString(buffer, context, header, tokenTable, operand);
		outInstructions.emplace_back(PRPInstruction(opCode, std::move(opVal)));
	}

	void prepareStringArray(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		const auto a0 = *reinterpret_cast<const int32_t *>(operand);

		if ((header->getFlags() >> 2) & 1) {
			if ((header->getFlags() >> 3) & 1) {
				// a0 is capacity

				StringArray stringArray;
				stringArray.resize(a0);

				for (int i = 0; i < a0; i++)
				{
					// Extract string by what?
					const auto la0 = *reinterpret_cast<const int32_t *>(&buffer[context.getIndex()]);

					auto val = exchangeString(buffer, context, header, tokenTable, reinterpret_cast<const uint8_t *>(&la0));
					stringArray[i] = std::move(val.str);

					context += 4;
				}

				PRPOperandVal val(std::move(stringArray));
				outInstructions.emplace_back(PRPInstruction(opCode, std::move(val)));
			} else {
				// Not implemented
				throw PRPOpCodeNotImplemented("prepareStringArray: this case not implemented yet (1:0)", PRPRegionID::INSTRUCTIONS, context.getIndex());
			}
		} else {
			// Not implemented
			throw PRPOpCodeNotImplemented("prepareStringArray: this case not implemented yet (0:0)", PRPRegionID::INSTRUCTIONS, context.getIndex());
		}
	}

	void prepareRawData(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		const auto length = *reinterpret_cast<const int32_t*>(operand);

		PRPOperandVal val;
		val.raw.resize(length);
		std::memcpy(&val.raw[0], &buffer[context.getIndex()], length);
		context += length; // Skip 'length' bytes

		outInstructions.emplace_back(PRPInstruction(opCode, std::move(val)));
	}

	void prepareSkipMark(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		outInstructions.emplace_back(PRPInstruction(opCode));
	}

	void prepareEnum(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
	{
		const auto a0 = *reinterpret_cast<const int32_t*>(operand);

		if ((header->getFlags() >> 2) & 1) // Value represented as token index
		{
			// Exchange string
			if (!tokenTable->hasIndex(a0))
			{
				throw PRPBadStringReference("String reference " + std::to_string(a0) + " is invalid!", PRPRegionID::INSTRUCTIONS, context.getIndex());
			}

			PRPOperandVal val(tokenTable->tokenAt(a0));
			outInstructions.emplace_back(PRPInstruction(opCode, std::move(val)));
		} else { // Value represented as integral value
			// Extract 4 bytes
			PRPOperandVal val(a0);
			outInstructions.emplace_back(PRPInstruction(opCode, std::move(val)));
		}
	}

	void prepareReference(const Span<uint8_t> &, PRPByteCodeContext &, PRPOpCode opCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &)
	{
		throw PRPOpCodeNotImplemented("OpCode " + to_string(opCode) + " not implemented yet!", PRPRegionID::INSTRUCTIONS, -1);
	}

	///----------------
	/// SERIALIZERS
	///----------------
	void serializeTrivial(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		const auto opCode = instruction.getOpCode();
		switch (opCode) {
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(instruction.getOperand().trivial.i8);
			break;
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
			binaryWriter->write<uint16_t, ZBio::Endianness::LE>(instruction.getOperand().trivial.i16);
			break;
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
		case PRPOpCode::Bitfield:
		case PRPOpCode::NameBitfield:
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(instruction.getOperand().trivial.i32);
			break;
		case PRPOpCode::Float32:
		case PRPOpCode::NamedFloat32:
			binaryWriter->write<float, ZBio::Endianness::LE>(instruction.getOperand().trivial.f32);
			break;
		case PRPOpCode::Float64:
		case PRPOpCode::NamedFloat64:
			binaryWriter->write<double, ZBio::Endianness::LE>(instruction.getOperand().trivial.f64);
			break;
		case PRPOpCode::Bool:
		case PRPOpCode::NamedBool:
			binaryWriter->write<bool, ZBio::Endianness::LE>(instruction.getOperand().trivial.b);
			break;
		case PRPOpCode::Char:
		case PRPOpCode::NamedChar:
			binaryWriter->write<char, ZBio::Endianness::LE>(instruction.getOperand().trivial.c);
			break;
		default:
			assert(false); // Unsupported
		}
	}

	void serializeString(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *tokenTable, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		auto tokenIndex = tokenTable->indexOf(instruction.getOperand().str);
		if (tokenIndex < 0)
		{
			throw PRPBadInstruction("Bad instruction! Token '" + instruction.getOperand().str + "' not found in token table!", PRPRegionID::INSTRUCTIONS, -1);
		}

		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenIndex);
	}

	void serializeEnum(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *tokenTable, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		auto tokenIndex = tokenTable->indexOf(instruction.getOperand().str);
		if (tokenIndex < 0)
		{
			throw PRPBadInstruction("Bad instruction! Token '" + instruction.getOperand().str + "' not found in token table!", PRPRegionID::INSTRUCTIONS, -1);
		}

		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenIndex);
	}

	void serializeRawData(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		const auto& raw = instruction.getOperand().raw;
		const auto length = raw.size();

		// Write length
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(length);

		// Write bytes array
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(&raw[0], length);
	}

	void serializeStringArray(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *tokenTable, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		// Length
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(instruction.getOperand().stringArray.size());

		// Entries
		for (const auto &entry: instruction.getOperand().stringArray)
		{
			auto tokenIndex = tokenTable->indexOf(entry);

			if (tokenIndex < 0)
			{
				throw PRPBadInstruction("Bad string-array instruction! Token '" + entry + "' not found!", PRPRegionID::INSTRUCTIONS, -1);
			}

			// Save string index
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenIndex);
		}
	}

	void serializeArrayOrContainer(const PRPInstruction &instruction, const PRPHeader *, const PRPTokenTable *, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		// Length
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(instruction.getOperand().trivial.i32);
	}
}