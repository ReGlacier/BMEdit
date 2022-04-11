#include <GameLib/PRPByteCode.h>
#include <GameLib/PRPStructureError.h>
#include <GameLib/PRPBadInstruction.h>
#include <GameLib/PRPOpCodeNotImplemented.h>
#include <GameLib/PRPBadStringReference.h>
#include <ZBinaryReader.hpp>
#include <cassert>


namespace gamelib::prp
{
	namespace opc
	{
		struct OpCodeHandler {
			using Handler = void(*)(const Span<uint8_t> &buffer, PRPByteCodeContext&, PRPOpCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &outInstructions);

			PRPOpCode opCode;
			int operandSize;
			Handler handler;

			constexpr OpCodeHandler(PRPOpCode _opCode, int _operandSize, Handler _handler)
				: opCode(_opCode)
				, operandSize(_operandSize)
				, handler(_handler)
			{
			}

			void operator()(const Span<uint8_t>& buffer, PRPByteCodeContext& context, const PRPHeader *header, const PRPTokenTable *tokenTable, std::vector<PRPInstruction> &outInstructions) const
			{
				constexpr int kMaxOperandSize = 16;

				uint8_t operandMemory[kMaxOperandSize] { 0 };
				uint8_t* operandDataPtr { nullptr };

				if (operandSize)
				{
					assert(operandSize <= kMaxOperandSize); // Not enough local size

					operandDataPtr = &operandMemory[0];

					// Extract N bytes from ctx and skip 'em
					std::memcpy(operandDataPtr, &buffer.data[context.getIndex()], operandSize);
					context += operandSize;
				}

				handler(buffer, context, opCode, header, tokenTable, operandDataPtr, outInstructions);
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

		static void prepareBeginObject(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			outInstructions.emplace_back(PRPInstruction(opCode));
			context.setFlag(PRPByteCodeContext::ContextFlags::CF_READ_OBJECT);
		}

		static void prepareEndOfStream(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			outInstructions.emplace_back(PRPInstruction(opCode));
			context.setFlag(PRPByteCodeContext::ContextFlags::CF_END_OF_STREAM);
		}

		static void prepareBool(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const bool*>(operand);

			PRPOperandVal operandVal(a0);
			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareChar(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const char*>(operand);

			PRPOperandVal operandVal(a0);
			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareInt8(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const int8_t*>(operand);
			PRPOperandVal operandVal(a0);

			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareInt16(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const int16_t*>(operand);
			PRPOperandVal operandVal(a0);

			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareInt32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const int32_t*>(operand);
			PRPOperandVal operandVal(a0);

			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareFloat32(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const float *>(operand);
			PRPOperandVal operandVal(a0);

			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static void prepareFloat64(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto a0 = *reinterpret_cast<const double *>(operand);
			PRPOperandVal operandVal(a0);

			outInstructions.emplace_back(PRPInstruction(opCode, operandVal));
		}

		static PRPOperandVal exchangeString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand)
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

		static void prepareString(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			auto opVal = exchangeString(buffer, context, header, tokenTable, operand);
			outInstructions.emplace_back(PRPInstruction(opCode, std::move(opVal)));
		}

		static void prepareStringArray(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
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

		static void prepareRawData(const Span<uint8_t> &buffer, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			const auto length = *reinterpret_cast<const int32_t*>(operand);

			PRPOperandVal val;
			val.raw.resize(length);
			std::memcpy(&val.raw[0], &buffer[context.getIndex()], length);
			context += length; // Skip 'length' bytes

			outInstructions.emplace_back(PRPInstruction(opCode, std::move(val)));
		}

		static void prepareSkipMark(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
		{
			outInstructions.emplace_back(PRPInstruction(opCode));
		}

		static void prepareEnum(const Span<uint8_t> &, PRPByteCodeContext &context, PRPOpCode opCode, const PRPHeader *header, const PRPTokenTable *tokenTable, const uint8_t *operand, std::vector<PRPInstruction> &outInstructions)
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

		static void prepareReference(const Span<uint8_t> &, PRPByteCodeContext &, PRPOpCode opCode, const PRPHeader *, const PRPTokenTable *, const uint8_t *, std::vector<PRPInstruction> &)
		{
			throw PRPOpCodeNotImplemented("OpCode " + to_string(opCode) + " not implemented yet!", PRPRegionID::INSTRUCTIONS, -1);
		}
	}

	bool PRPByteCode::parse(const uint8_t *data, int64_t size, const PRPHeader *header, const PRPTokenTable *tokenTable)
	{
		if (!data || !size  || !header || !tokenTable)
		{
			/// Maybe later we will support work without token table but not now
			return false;
		}

		m_buffer.data = data;
		m_buffer.size = size;

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
		if (opCode == PRPOpCode::ERR_UNKNOWN || opCode == PRPOpCode::ERR_NO_TAG)
		{
			throw PRPBadInstruction("Invalid instruction", PRPRegionID::INSTRUCTIONS, context.getIndex());
		}
		context += 1; // Skip ready opcode

		static constexpr opc::OpCodeHandler handlers[] = {
			{ PRPOpCode::Array, 4, opc::prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY> },
			{ PRPOpCode::NamedArray, 4, opc::prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY> },
			{ PRPOpCode::EndArray, 0, opc::prepareEndOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_ARRAY> },
			{ PRPOpCode::BeginObject, 0, opc::prepareBeginObject },
			{ PRPOpCode::BeginNamedObject, 0, opc::prepareBeginObject },
			{ PRPOpCode::EndObject, 0, opc::prepareEndOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_OBJECT> },
			{ PRPOpCode::Container, 4, opc::prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_CONTAINER> },
			{ PRPOpCode::NamedContainer, 4, opc::prepareBeginOfGenericContainer<PRPByteCodeContext::ContextFlags::CF_READ_CONTAINER> },
			{ PRPOpCode::EndOfStream, 0, opc::prepareEndOfStream },
			{ PRPOpCode::Bool, 1, opc::prepareBool },
			{ PRPOpCode::Char, 1, opc::prepareChar },
			{ PRPOpCode::Int8, 1, opc::prepareInt8 },
			{ PRPOpCode::NamedInt8, 1, opc::prepareInt8 },
			{ PRPOpCode::Int16, 2, opc::prepareInt16 },
			{ PRPOpCode::NamedInt16, 2, opc::prepareInt16 },
			{ PRPOpCode::Int32, 4, opc::prepareInt32 },
			{ PRPOpCode::NamedInt32, 4, opc::prepareInt32 },
			{ PRPOpCode::Float32, 4, opc::prepareFloat32 },
			{ PRPOpCode::NamedFloat32, 4, opc::prepareFloat32 },
			{ PRPOpCode::Float64, 8, opc::prepareFloat64 },
			{ PRPOpCode::NamedFloat64, 8, opc::prepareFloat64 },
			{ PRPOpCode::Bitfield, 4, opc::prepareInt32 },
			{ PRPOpCode::NameBitfield, 4, opc::prepareInt32 },
			{ PRPOpCode::String, 4, opc::prepareString },
			{ PRPOpCode::NamedString, 4, opc::prepareString },
			{ PRPOpCode::StringArray, 4, opc::prepareStringArray },
			{ PRPOpCode::RawData, 4, opc::prepareRawData },
			{ PRPOpCode::NamedRawData, 4, opc::prepareRawData },
			{ PRPOpCode::SkipMark, 0, opc::prepareSkipMark },
			{ PRPOpCode::StringOrArray_E, 4, opc::prepareEnum },
			{ PRPOpCode::StringOrArray_8E, 4, opc::prepareEnum },
			{ PRPOpCode::Reference, 0, opc::prepareReference }, // Not implemented
			{ PRPOpCode::NamedReference, 0, opc::prepareReference }, // Not implemented
		};

		for (const auto& handler: handlers)
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
}