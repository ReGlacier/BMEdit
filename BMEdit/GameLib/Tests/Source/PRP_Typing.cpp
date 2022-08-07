#include <gtest/gtest.h>

#include <GameLib/PRP/PRP.h>
#include <GameLib/PRP/PRPHeader.h>
#include <GameLib/PRP/PRPReader.h>
#include <GameLib/PRP/PRPWriter.h>
#include <GameLib/PRP/PRPTokenTable.h>

#include <GameLib/Type.h>
#include <GameLib/TypeEnum.h>
#include <GameLib/TypeArray.h>
#include <GameLib/TypeAlias.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeRegistry.h>

// Usage
using gamelib::Type;
using gamelib::TypeEnum;
using gamelib::TypeArray;
using gamelib::TypeAlias;
using gamelib::TypeComplex;
using gamelib::TypeRegistry;
using gamelib::prp::PRPReader;
using gamelib::prp::PRPWriter;
using gamelib::prp::PRPHeader;
using gamelib::prp::PRPTokenTable;
using gamelib::prp::PRPByteCode;
using gamelib::prp::PRPOpCode;

// Fixture
class PRP_Typing : public ::testing::Test
{
protected:
	void SetUp() override
	{
		TypeRegistry::getInstance().reset();

		// Register ZVector3F
		{
			TypeRegistry::getInstance().registerType(std::make_unique<TypeArray>("ZVector3F", PRPOpCode::Float32, 3));
		}

		// Register ZMatrix33F
		{
			TypeRegistry::getInstance().registerType(std::make_unique<TypeArray>("ZMatrix33F", PRPOpCode::Float32, 9));
		}

		// Register EBoundingBox
		{
			TypeEnum::Entries entries;
			entries.emplace_back("BOUNDING_Static", 0);
			entries.emplace_back("BOUNDING_Dynamic", 1);
			entries.emplace_back("BOUNDING_DynamicAutoAssign", 2);

			auto type = std::make_unique<TypeEnum>("EBoundingBox", entries);
			TypeRegistry::getInstance().registerType(std::move(type));
		}

		// Register sample type ZGEOM
		{
			std::vector<gamelib::ValueView> views;

			views.emplace_back(gamelib::ValueView("BoundingBox", "EBoundingBox", nullptr));
			views.emplace_back(gamelib::ValueView("Matrix", "ZMatrix33F", nullptr));
			views.emplace_back(gamelib::ValueView("Position", "ZVector3F", nullptr));
			views.emplace_back(gamelib::ValueView("IsInactive", PRPOpCode::Bool, nullptr));
			views.emplace_back(gamelib::ValueView("PrimId", PRPOpCode::Int32, nullptr));

			auto type = std::make_unique<TypeComplex>("ZGEOM", std::move(views), nullptr, false);
			TypeRegistry::getInstance().registerType(std::move(type));
		}

		// Register type ZSTDOBJ
		{
			std::vector<gamelib::ValueView> views;
			views.emplace_back(gamelib::ValueView("Invisible", PRPOpCode::Bool, nullptr));

			auto geom = TypeRegistry::getInstance().findTypeByName("ZGEOM");
			ASSERT_NE(geom, nullptr) << "ZGEOM not registered!";

			auto type = std::make_unique<TypeComplex>("ZSTDOBJ", std::move(views), const_cast<Type*>(geom), false);
			TypeRegistry::getInstance().registerType(std::move(type));
		}

		TypeRegistry::getInstance().linkTypes();
	}

	void TearDown() override
	{
		TypeRegistry::getInstance().reset();
	}
};

// Tests
TEST_F(PRP_Typing, SimpleTypeRecognition)
{
	const uint8_t kByteCode[] = {
	    // Matrix represented as StringOrArray_E or StringOrArray_8E op-code
	    (uint8_t)(PRPOpCode::StringOrArray_E), 0x00, 0x00, 0x00, 0x00, // Operand is index of string in token table

	    // Matrix represented as BeginArray + 4 bytes capacity + N entries + EndArray
	    (uint8_t)(PRPOpCode::Array), 0x09, 0x00, 0x00, 0x00, //9 entries

	    // First row
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,

	    // Second row
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    // Third row
		    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
		    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    (uint8_t)(PRPOpCode::EndArray),
	    // Vector represented as BeginArray + Int32 cap + N entries + EndArray
	    (uint8_t)(PRPOpCode::Array), 0x03, 0x00, 0x00, 0x00, // 3 entries

		    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x99, 0x99, 0x9A, //X = 1.2f
		    (uint8_t)(PRPOpCode::Float32), 0x41, 0x20, 0x00, 0x00, //Y = 10.f
		    (uint8_t)(PRPOpCode::Float32), 0xC0, 0xA8, 0x00, 0x00, //Z = -5.25f

	    (uint8_t)(PRPOpCode::EndArray),
	    // IsInactive - PRPOpCode::Bool
	    (uint8_t)(PRPOpCode::Bool), 0x1,
	    // PrimId - PRPOpCode::Int32
	    (uint8_t)(PRPOpCode::Int32), 0x05, 0x00, 0x00, 0x00,

	    // --- END OF STREAM ---
	    (uint8_t)(PRPOpCode::EndOfStream)
	};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("BOUNDING_DynamicAutoAssign");

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	// Now we are ready to map raw data to views
	auto geomType = TypeRegistry::getInstance().findTypeByName("ZGEOM");
	ASSERT_NE(geomType, nullptr);

	const auto& [value, newSpan] = geomType->map(gamelib::Span<gamelib::prp::PRPInstruction>(byteCode.getInstructions()));
	ASSERT_TRUE(value.has_value());

	const auto& entries = value.value().getEntries();
	ASSERT_EQ(entries.size, 5);

	const Type* pVector3FType = TypeRegistry::getInstance().findTypeByName("ZVector3F");
	const Type* pZMatrix33FType = TypeRegistry::getInstance().findTypeByName("ZMatrix33F");
	const Type* pEBoundingBoxType = TypeRegistry::getInstance().findTypeByName("EBoundingBox");

	ASSERT_EQ(entries[0].instructions.size(), 1);
	ASSERT_EQ(entries[0].views.size(), 1);
	ASSERT_EQ(entries[0].name, "BoundingBox");
	ASSERT_EQ(entries[0].views[0].getType(), pEBoundingBoxType);

	ASSERT_EQ(entries[1].instructions.size(), 11);
	ASSERT_EQ(entries[1].views.size(), 1);
	ASSERT_EQ(entries[1].name, "Matrix");
	ASSERT_EQ(entries[1].views[0].getType(), pZMatrix33FType);

	ASSERT_EQ(entries[2].instructions.size(), 5);
	ASSERT_EQ(entries[2].views.size(), 1);
	ASSERT_EQ(entries[2].name, "Position");
	ASSERT_EQ(entries[2].views[0].getType(), pVector3FType);

	ASSERT_EQ(entries[3].instructions.size(), 1);
	ASSERT_EQ(entries[3].views.size(), 1);
	ASSERT_EQ(entries[3].name, "IsInactive");
	ASSERT_TRUE(entries[3].views[0].isTrivialType());
	ASSERT_EQ(entries[3].views[0].getTrivialType(), PRPOpCode::Bool);

	ASSERT_EQ(entries[4].instructions.size(), 1);
	ASSERT_EQ(entries[4].views.size(), 1);
	ASSERT_EQ(entries[4].name, "PrimId");
	ASSERT_TRUE(entries[4].views[0].isTrivialType());
	ASSERT_EQ(entries[4].views[0].getTrivialType(), PRPOpCode::Int32);

	ASSERT_TRUE(newSpan);
	ASSERT_EQ(newSpan.size, 1);
	ASSERT_EQ(newSpan[0].getOpCode(), PRPOpCode::EndOfStream);
}

TEST_F(PRP_Typing, TypeRecognitionWithInheritance)
{
	const uint8_t kByteCode[] = {
	    // Matrix represented as StringOrArray_E or StringOrArray_8E op-code
	    (uint8_t)(PRPOpCode::StringOrArray_E), 0x00, 0x00, 0x00, 0x00, // Operand is index of string in token table

	    // Matrix represented as BeginArray + 4 bytes capacity + N entries + EndArray
	    (uint8_t)(PRPOpCode::Array), 0x09, 0x00, 0x00, 0x00, //9 entries

	    // First row
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,

	    // Second row
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    // Third row
	    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t)(PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    (uint8_t)(PRPOpCode::EndArray),
	    // Vector represented as BeginArray + Int32 cap + N entries + EndArray
	    (uint8_t)(PRPOpCode::Array), 0x03, 0x00, 0x00, 0x00, // 3 entries

	    (uint8_t)(PRPOpCode::Float32), 0x3F, 0x99, 0x99, 0x9A, //X = 1.2f
	    (uint8_t)(PRPOpCode::Float32), 0x41, 0x20, 0x00, 0x00, //Y = 10.f
	    (uint8_t)(PRPOpCode::Float32), 0xC0, 0xA8, 0x00, 0x00, //Z = -5.25f

	    (uint8_t)(PRPOpCode::EndArray),

	    // IsInactive - PRPOpCode::Bool
	    (uint8_t)(PRPOpCode::Bool), 0x1,

	    // PrimId - PRPOpCode::Int32
	    (uint8_t)(PRPOpCode::Int32), 0x05, 0x00, 0x00, 0x00,

	    // Invisible - PRPOpCode::Bool
	    (uint8_t)(PRPOpCode::Bool), 0x0,

	    // --- END OF STREAM ---
	    (uint8_t)(PRPOpCode::EndOfStream)
	};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("BOUNDING_DynamicAutoAssign");

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	// Now we are ready to map raw data to views
	auto stdobjType = TypeRegistry::getInstance().findTypeByName("ZSTDOBJ");
	ASSERT_NE(stdobjType, nullptr);

	auto geomType = TypeRegistry::getInstance().findTypeByName("ZGEOM");
	ASSERT_NE(geomType, nullptr);

	const Type* pVector3FType = TypeRegistry::getInstance().findTypeByName("ZVector3F");
	ASSERT_NE(pVector3FType, nullptr);

	const Type* pZMatrix33FType = TypeRegistry::getInstance().findTypeByName("ZMatrix33F");
	ASSERT_NE(pZMatrix33FType, nullptr);

	const Type* pEBoundingBoxType = TypeRegistry::getInstance().findTypeByName("EBoundingBox");
	ASSERT_NE(pEBoundingBoxType, nullptr);

	const auto& [value, newSpan] = stdobjType->map(gamelib::Span<gamelib::prp::PRPInstruction>(byteCode.getInstructions()));
	ASSERT_TRUE(value.has_value());

	const auto& entries = value->getEntries();
	ASSERT_EQ(entries.size, 6);

	ASSERT_EQ(entries[0].instructions.size(), 1);
	ASSERT_EQ(entries[0].views.size(), 1);
	ASSERT_EQ(entries[0].name, "BoundingBox");
	ASSERT_EQ(entries[0].views[0].getType(), pEBoundingBoxType);
	ASSERT_EQ(entries[0].views[0].getOwnerType(), geomType);

	ASSERT_EQ(entries[1].instructions.size(), 11);
	ASSERT_EQ(entries[1].views.size(), 1);
	ASSERT_EQ(entries[1].name, "Matrix");
	ASSERT_EQ(entries[1].views[0].getType(), pZMatrix33FType);
	ASSERT_EQ(entries[1].views[0].getOwnerType(), geomType);

	ASSERT_EQ(entries[2].instructions.size(), 5);
	ASSERT_EQ(entries[2].views.size(), 1);
	ASSERT_EQ(entries[2].name, "Position");
	ASSERT_EQ(entries[2].views[0].getType(), pVector3FType);
	ASSERT_EQ(entries[2].views[0].getOwnerType(), geomType);

	ASSERT_EQ(entries[3].instructions.size(), 1);
	ASSERT_EQ(entries[3].views.size(), 1);
	ASSERT_EQ(entries[3].name, "IsInactive");
	ASSERT_TRUE(entries[3].views[0].isTrivialType());
	ASSERT_EQ(entries[3].views[0].getTrivialType(), PRPOpCode::Bool);
	ASSERT_EQ(entries[3].views[0].getOwnerType(), geomType);

	ASSERT_EQ(entries[4].instructions.size(), 1);
	ASSERT_EQ(entries[4].views.size(), 1);
	ASSERT_EQ(entries[4].name, "PrimId");
	ASSERT_TRUE(entries[4].views[0].isTrivialType());
	ASSERT_EQ(entries[4].views[0].getTrivialType(), PRPOpCode::Int32);
	ASSERT_EQ(entries[4].views[0].getOwnerType(), geomType);

	ASSERT_EQ(entries[5].instructions.size(), 1);
	ASSERT_EQ(entries[5].views.size(), 1);
	ASSERT_EQ(entries[5].name, "Invisible");
	ASSERT_TRUE(entries[5].views[0].isTrivialType());
	ASSERT_EQ(entries[5].views[0].getTrivialType(), PRPOpCode::Bool);
	ASSERT_EQ(entries[5].views[0].getOwnerType(), stdobjType);

	ASSERT_TRUE(newSpan);
	ASSERT_EQ(newSpan.size, 1);
	ASSERT_EQ(newSpan[0].getOpCode(), PRPOpCode::EndOfStream);

}