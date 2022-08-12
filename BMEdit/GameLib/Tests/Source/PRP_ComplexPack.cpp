#include <gtest/gtest.h>

#include <GameLib/Span.h>

#include <GameLib/Scene/SceneObject.h>
#include <GameLib/Scene/SceneObjectPropertiesLoader.h>

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
using gamelib::Span;
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
using gamelib::scene::SceneObject;
using gamelib::scene::SceneObjectPropertiesLoader;


class PRP_ComplexPack : public ::testing::Test
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

		// Register linkage to type 0x200002
		TypeRegistry::getInstance().addHashAssociation(0x200002, "ZSTDOBJ");

		// Register global links
		TypeRegistry::getInstance().linkTypes();
	}

	void TearDown() override
	{
		TypeRegistry::getInstance().reset();
	}
};

TEST_F(PRP_ComplexPack, TrivialDecl)
{
	const uint8_t kByteCode[] = {
	    // Begin Object
	    (uint8_t)(PRPOpCode::BeginObject),

	    // BoundingBox represented as StringOrArray_E or StringOrArray_8E op-code
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

	    // (ZSTDOBJ) - Invisible
	    (uint8_t)(PRPOpCode::Bool), 0x0,

	    // End Object
	    (uint8_t)(PRPOpCode::EndObject),

	    // Controllers (no controllers)
	    (uint8_t)(PRPOpCode::Container), 0x00, 0x00, 0x00, 0x00,

	    // Children (no children)
	    (uint8_t)(PRPOpCode::Container), 0x00, 0x00, 0x00, 0x00,

	    // --- END OF STREAM ---
	    (uint8_t)(PRPOpCode::EndOfStream)
	};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("BOUNDING_DynamicAutoAssign");

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	// Then create list of scene objects (just one object for this test)
	std::vector<SceneObject::Ptr> sceneObjects;

	{
		gamelib::gms::GMSGeomEntity geomInfo;
		SceneObject::Instructions instructions;

		sceneObjects = {
		    std::make_shared<SceneObject>("Hero", 0x200002, TypeRegistry::getInstance().findTypeByName("ZSTDOBJ"), std::move(geomInfo), std::move(instructions))
        };
	}

	// And then we may to map data to scene entities
	const auto& instructions = byteCode.getInstructions();
	Span ip { instructions };
	ASSERT_NO_THROW(SceneObjectPropertiesLoader::load(Span(sceneObjects), ip));

	//TODO: Check type
	//TODO: Check properties
}

//TODO: Check controllers (1 or multiple)
//TODO: Check children iteration (1 children, 2 children, two parallel objects with abstract ROOT)