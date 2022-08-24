#include <GameLib/PRP/PRPZDefines.h>
#include <GameLib/PRP/PRPOpCode.h>
#include <GameLib/PRP/PRPStructureError.h>
#include <GameLib/PRP/PRPBadStringReference.h>
#include <GameLib/PRP/PRPDefinitionType.h>
#include <ZBinaryReader.hpp>
#include <ZBinaryWriter.hpp>


namespace gamelib::prp
{
	void PRPZDefines::read(const uint8_t *data,
	                       int64_t size,
	                       const PRPTokenTable *tokenTable,
	                       PRPZDefines::ReadResult &result)
	{
		if (!tokenTable) {
			throw std::runtime_error("Unsupported work without token table!");
		}

		ZBio::ZBinaryReader::BinaryReader binaryReader(reinterpret_cast<const char *>(data), size);
		auto rootContainerOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
		if (rootContainerOpCode != PRPOpCode::Container) {
			throw PRPStructureError("Invalid root opcode, expected container, got " + to_string(rootContainerOpCode),
			                        PRPRegionID::ZDEFINITIONS,
			                        binaryReader.tell());
		}

		auto entriesCount = binaryReader.read<int32_t, ZBio::Endianness::LE>();
		if (entriesCount <= 0) {
			throw PRPStructureError("Bad ZDefs count in PRP file!", PRPRegionID::ZDEFINITIONS, binaryReader.tell());
		}

		for (int entryIndex = 0; entryIndex < entriesCount; entryIndex++) {
			// Read ZDef name
			auto entryNameOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
			if (entryNameOpCode != PRPOpCode::String) {
				throw PRPStructureError("Expected String opcode, got " + to_string(entryNameOpCode),
				                        PRPRegionID::ZDEFINITIONS,
				                        binaryReader.tell());
			}

			auto entryNameIndex = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
			if (!tokenTable->hasIndex(entryNameIndex)) {
				throw PRPBadStringReference("Invalid string reference " + std::to_string(entryNameIndex),
				                            PRPRegionID::ZDEFINITIONS,
				                            binaryReader.tell());
			}

			const auto &entryName = tokenTable->tokenAt(entryNameIndex);

			// Read ZDef kind
			auto defKindOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
			if (defKindOpCode != PRPOpCode::Int32) {
				throw PRPStructureError("Expected Int32, got " + to_string(defKindOpCode),
				                        PRPRegionID::ZDEFINITIONS,
				                        binaryReader.tell());
			}

			auto defKindValue = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
			auto defKind =
				FromBytes<PRPDefinitionType>()(defKindValue); //? Implicit cast from uint32_t -> uint8_t is not a good idea, will fix it later

			if (defKind == PRPDefinitionType::ERR_UNKNOWN) {
				throw PRPStructureError("Unknown ZDefinition kind " + std::to_string(defKindValue),
				                        PRPRegionID::ZDEFINITIONS,
				                        binaryReader.tell());
			}

			auto &definitionEntry = m_definitions.emplace_back();
			definitionEntry.setName(entryName);

			switch (defKind) {
			case PRPDefinitionType::Array_Int32: {
				ArrayI32 arrayI32;
				// Read
				auto lengthOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (lengthOpCode != PRPOpCode::Int32) {
					throw PRPStructureError("Expected Int32-length opcode, got " + to_string(lengthOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto capacity = binaryReader.read<uint32_t, ZBio::Endianness::LE>();

				// BeginArray
				auto beginArrayOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (beginArrayOpCode != PRPOpCode::Array) {
					throw PRPStructureError("Expected Array, got " + to_string(beginArrayOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto arrayCapacity = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
				if (capacity != arrayCapacity) {
					throw PRPStructureError(
						"Expected same capacity of pre-declaration and array op code but it's different ("
							+ std::to_string(capacity) + " vs " + std::to_string(arrayCapacity) + ")",
						PRPRegionID::ZDEFINITIONS,
						binaryReader.tell());
				}

				arrayI32.resize(arrayCapacity);
				for (int arrEntryIdx = 0; arrEntryIdx < arrayCapacity; arrEntryIdx++) {
					auto &value = arrayI32[arrEntryIdx];

					auto i32opCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
					if (i32opCode != PRPOpCode::Int32) {
						throw PRPStructureError("Expected Int32, got " + to_string(i32opCode),
						                        PRPRegionID::ZDEFINITIONS,
						                        binaryReader.tell());
					}

					value = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
				}

				// Check for EndArray op-code
				auto endArrayOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (endArrayOpCode != PRPOpCode::EndArray) {
					throw PRPStructureError("Expected EndArray, got " + to_string(endArrayOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				// save
				definitionEntry.setValue(arrayI32, defKind);
			}
				break;
			case PRPDefinitionType::Array_Float32: {
				ArrayF32 arrayF32;
				// Read
				auto lengthOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (lengthOpCode != PRPOpCode::Int32) {
					throw PRPStructureError("Expected Int32-length opcode, got " + to_string(lengthOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto capacity = binaryReader.read<uint32_t, ZBio::Endianness::LE>();

				// BeginArray
				auto beginArrayOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (beginArrayOpCode != PRPOpCode::Array) {
					throw PRPStructureError("Expected Array, got " + to_string(beginArrayOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto arrayCapacity = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
				if (capacity != arrayCapacity) {
					throw PRPStructureError(
						"Expected same capacity of pre-declaration and array op code but it's different ("
							+ std::to_string(capacity) + " vs " + std::to_string(arrayCapacity) + ")",
						PRPRegionID::ZDEFINITIONS,
						binaryReader.tell());
				}

				arrayF32.resize(arrayCapacity);
				for (int arrEntryIdx = 0; arrEntryIdx < arrayCapacity; arrEntryIdx++) {
					auto &value = arrayF32[arrEntryIdx];

					auto i32opCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
					if (i32opCode != PRPOpCode::Float32) {
						throw PRPStructureError("Expected Float32, got " + to_string(i32opCode),
						                        PRPRegionID::ZDEFINITIONS,
						                        binaryReader.tell());
					}

					value = binaryReader.read<float, ZBio::Endianness::LE>();
				}

				// Check for EndArray op-code
				auto endArrayOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (endArrayOpCode != PRPOpCode::EndArray) {
					throw PRPStructureError("Expected EndArray, got " + to_string(endArrayOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				// save
				definitionEntry.setValue(arrayF32, defKind);
			}
				break;
			case PRPDefinitionType::StringRef_1:
			case PRPDefinitionType::StringRef_2:
			case PRPDefinitionType::StringRef_3: {
				auto stringOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());

				if (stringOpCode != PRPOpCode::String) {
					throw PRPStructureError("Expected String, got " + to_string(stringOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto stringIndex = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
				if (!tokenTable->hasIndex(stringIndex)) {
					throw PRPBadStringReference("Bad string reference to " + std::to_string(stringIndex),
					                            PRPRegionID::ZDEFINITIONS,
					                            binaryReader.tell());
				}

				definitionEntry.setValue(tokenTable->tokenAt(stringIndex), defKind);
			}
				break;
			case PRPDefinitionType::StringRefTab: {
				StringRefTab refTab;

				auto containerOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
				if (containerOpCode != PRPOpCode::Container) {
					throw PRPStructureError("Expected Container, got " + to_string(containerOpCode),
					                        PRPRegionID::ZDEFINITIONS,
					                        binaryReader.tell());
				}

				auto capacity = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
				refTab.reserve(capacity);

				for (int strRefTabEntryIndex = 0; strRefTabEntryIndex < capacity; strRefTabEntryIndex++) {
					auto stringOpCode = FromBytes<PRPOpCode>()(binaryReader.read<uint8_t, ZBio::Endianness::LE>());
					if (stringOpCode != PRPOpCode::String) {
						throw PRPStructureError("Expected String, got " + to_string(stringOpCode),
						                        PRPRegionID::ZDEFINITIONS,
						                        binaryReader.tell());
					}

					/// NOTE: Here we reading contents by index, if here required support of other things you should use this code as base
					/// https://github.com/ReGlacier/HBM_GMSTool/blob/main/PRP/PRPReader.py#L206
					auto tokenIndex = binaryReader.read<uint32_t, ZBio::Endianness::LE>();
					if (!tokenTable->hasIndex(tokenIndex)) {
						throw PRPBadStringReference("Bad string reference",
						                            PRPRegionID::ZDEFINITIONS,
						                            binaryReader.tell());
					}

					refTab[strRefTabEntryIndex] = tokenTable->tokenAt(tokenIndex);
				}

				// Store
				definitionEntry.setValue(refTab, defKind);
			}
				break;
			default:
				throw PRPStructureError("Unknown ZDefinition kind " + std::to_string(defKindValue),
				                        PRPRegionID::ZDEFINITIONS,
				                        binaryReader.tell());
			}
		}

		// Save difference between iter and data - this is offset
		// Ending
		result.lastOffset = binaryReader.tell();
		result.isOk = true;
	}

	const std::vector<PRPDefinition> &PRPZDefines::getDefinitions() const
	{
		return m_definitions;
	}

	std::vector<PRPDefinition> &PRPZDefines::getDefinitions()
	{
		return m_definitions;
	}

	bool PRPZDefines::operator==(const PRPZDefines &other) const
	{
		return m_definitions == other.m_definitions;
	}

	bool PRPZDefines::operator!=(const PRPZDefines &other) const
	{
		return m_definitions != other.m_definitions;
	}

	void PRPZDefines::serialize(const PRPZDefines &defines,
	                            const PRPTokenTable *tokenTable,
	                            ZBio::ZBinaryWriter::BinaryWriter *binaryWriter)
	{
		// BeginContainer
		binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Container));

		// Capacity
		binaryWriter->write<uint32_t, ZBio::Endianness::LE>(defines.m_definitions.size());

		// For each object
		for (const auto &def: defines.m_definitions) {
			//      String
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::String));

			//      StringIndex
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenTable->indexOf(def.getName()));

			//      Int32
			binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Int32));

			//      Kind
			binaryWriter->write<uint32_t, ZBio::Endianness::LE>(static_cast<uint32_t>(def.getType()));

			//      Data
			switch (def.getType()) {
			case PRPDefinitionType::StringRef_1:
			case PRPDefinitionType::StringRef_2:
			case PRPDefinitionType::StringRef_3: {
				// String
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::String));

				// Value
				auto str = std::get_if<StringRef>(&def.getValue());
				assert(str != nullptr);

				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenTable->indexOf(*str));
			}
				break;
			case PRPDefinitionType::Array_Int32: {
				const auto& arr = *std::get_if<ArrayI32>(&def.getValue());

				// Int32
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Int32));

				// Capacity
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(arr.size());

				// BeginArray
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Array));

				// Capacity
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(arr.size());

				for (const auto& entry: arr)
				{
					//For entry
					//      Int32
					binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Int32));

					//      entry
					binaryWriter->write<uint32_t, ZBio::Endianness::LE>(entry);
				}

				// EndArray
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::EndArray));
			}
				break;
			case PRPDefinitionType::Array_Float32: {
				const auto& arr = *std::get_if<ArrayF32>(&def.getValue());

				// Int32
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Int32));

				// Capacity
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(arr.size());

				// BeginArray
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Array));

				// Capacity
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(arr.size());

				for (const auto& entry: arr)
				{
					//For entry
					//      Int32
					binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Float32));

					//      entry
					binaryWriter->write<float, ZBio::Endianness::LE>(entry);
				}

				// EndArray
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::EndArray));
			}
				break;
			case PRPDefinitionType::StringRefTab: {
				const auto& stringRefTab = *std::get_if<StringRefTab>(&def.getValue());
				// Container
				binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::Container));

				// Capacity
				binaryWriter->write<uint32_t, ZBio::Endianness::LE>(stringRefTab.size());

				// For entry
				for (const auto& entry: stringRefTab)
				{
					// Check that string contains index
					assert(tokenTable->hasToken(entry));

					//String
					binaryWriter->write<uint8_t, ZBio::Endianness::LE>(static_cast<uint8_t>(PRPOpCode::String));

					//StringIndex
					binaryWriter->write<uint32_t, ZBio::Endianness::LE>(tokenTable->indexOf(entry));
				}
			}
				break;
			}
		}
	}
}