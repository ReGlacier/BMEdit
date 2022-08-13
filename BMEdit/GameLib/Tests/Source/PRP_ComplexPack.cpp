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

		// Register type ZEventBase without any props
		{
			std::vector<gamelib::ValueView> views;

			auto eventBase = std::make_unique<TypeComplex>("ZEventBase", std::move(views), nullptr, false);
			TypeRegistry::getInstance().registerType(std::move(eventBase));
		}

		// Register type CInventory with parent ZEventBase
		{
			std::vector<gamelib::ValueView> views;
			views.emplace_back(gamelib::ValueView("Money", PRPOpCode::Int32, nullptr));

			auto eventBase = TypeRegistry::getInstance().findTypeByName("ZEventBase");
			ASSERT_NE(eventBase, nullptr) << "ZEventBase not registered!";

			auto inventory = std::make_unique<TypeComplex>("CInventory", std::move(views), const_cast<Type*>(eventBase), false);
			TypeRegistry::getInstance().registerType(std::move(inventory));
		}

		// Register type ZTie
		{
			std::vector<gamelib::ValueView> views;
			views.emplace_back(gamelib::ValueView("IsVisible", PRPOpCode::Bool, nullptr));
			views.emplace_back(gamelib::ValueView("Multiplier", PRPOpCode::Int32, nullptr));

			auto eventBase = TypeRegistry::getInstance().findTypeByName("ZEventBase");
			ASSERT_NE(eventBase, nullptr);

			auto tie = std::make_unique<TypeComplex>("ZTie", std::move(views), const_cast<Type*>(eventBase), false);
			TypeRegistry::getInstance().registerType(std::move(tie));
		}

		// Register ZScriptC (unexposed type)
		{
			std::vector<gamelib::ValueView> views;
			views.emplace_back(gamelib::ValueView("ScriptName", PRPOpCode::String, nullptr));

			auto eventBase = TypeRegistry::getInstance().findTypeByName("ZEventBase");
			ASSERT_NE(eventBase, nullptr) << "ZEventBase not registered!";

			auto type = std::make_unique<TypeComplex>("ZScriptC", std::move(views), const_cast<Type*>(eventBase), true); // Allow unexposed instructions
			TypeRegistry::getInstance().registerType(std::move(type));
		}

		// Register linkage to type 0x200002
		TypeRegistry::getInstance().addHashAssociation(0x0, "ZGEOM"); // ROOT type
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

	ASSERT_TRUE(sceneObjects[0]->getControllers().empty());

	ASSERT_EQ(sceneObjects[0]->getProperties().getEntries().size, 6);

	auto stdobjType = TypeRegistry::getInstance().findTypeByName("ZSTDOBJ");
	ASSERT_NE(stdobjType, nullptr);

	ASSERT_EQ(sceneObjects[0]->getType(), stdobjType);

	auto geomType = TypeRegistry::getInstance().findTypeByName("ZGEOM");
	ASSERT_NE(geomType, nullptr);

	const Type* pVector3FType = TypeRegistry::getInstance().findTypeByName("ZVector3F");
	ASSERT_NE(pVector3FType, nullptr);

	const Type* pZMatrix33FType = TypeRegistry::getInstance().findTypeByName("ZMatrix33F");
	ASSERT_NE(pZMatrix33FType, nullptr);

	const Type* pEBoundingBoxType = TypeRegistry::getInstance().findTypeByName("EBoundingBox");
	ASSERT_NE(pEBoundingBoxType, nullptr);

	const auto& entries = sceneObjects[0]->getProperties().getEntries();
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
}

TEST_F(PRP_ComplexPack, DeclWithController)
{
	const uint8_t kByteCode[] = {
	    // Begin Object
	    (uint8_t) (PRPOpCode::BeginObject),

	    // BoundingBox represented as StringOrArray_E or StringOrArray_8E op-code
	    (uint8_t) (PRPOpCode::StringOrArray_E), 0x00, 0x00, 0x00, 0x00,// Operand is index of string in token table

	    // Matrix represented as BeginArray + 4 bytes capacity + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x09, 0x00, 0x00, 0x00,//9 entries

	    // First row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,

	    // Second row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    // Third row
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    (uint8_t) (PRPOpCode::EndArray),
	    // Vector represented as BeginArray + Int32 cap + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x03, 0x00, 0x00, 0x00,// 3 entries

	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x99, 0x99, 0x9A,//X = 1.2f
	    (uint8_t) (PRPOpCode::Float32), 0x41, 0x20, 0x00, 0x00,//Y = 10.f
	    (uint8_t) (PRPOpCode::Float32), 0xC0, 0xA8, 0x00, 0x00,//Z = -5.25f

	    (uint8_t) (PRPOpCode::EndArray),

	    // IsInactive - PRPOpCode::Bool
	    (uint8_t) (PRPOpCode::Bool), 0x1,

	    // PrimId - PRPOpCode::Int32
	    (uint8_t) (PRPOpCode::Int32), 0x05, 0x00, 0x00, 0x00,

	    // (ZSTDOBJ) - Invisible
	    (uint8_t) (PRPOpCode::Bool), 0x0,

	    // End Object
	    (uint8_t) (PRPOpCode::EndObject),

	    // Controllers (1)
	    (uint8_t) (PRPOpCode::Container), 0x01, 0x00, 0x00, 0x00,

	    // Controller[0]
	    (uint8_t)(PRPOpCode::String), 0x01, 0x00, 0x00, 0x00, // CInventory (#1)
	    (uint8_t)(PRPOpCode::BeginObject),
	    (uint8_t)(PRPOpCode::Int32), 0x2A, 0x00, 0x00, 0x00, // CInventory::Money (0x2A => 42 coins)
	    (uint8_t)(PRPOpCode::EndObject),

	    // Children (no children)
	    (uint8_t) (PRPOpCode::Container), 0x00, 0x00, 0x00, 0x00,

	    // --- END OF STREAM ---
	    (uint8_t) (PRPOpCode::EndOfStream)};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("BOUNDING_DynamicAutoAssign"); // #0
	tokenTable.addToken("Inventory"); //#1

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	// Then create list of scene objects (just one object for this test)
	std::vector<SceneObject::Ptr> sceneObjects;

	{
		gamelib::gms::GMSGeomEntity geomInfo;
		SceneObject::Instructions instructions;

		sceneObjects = {
		    std::make_shared<SceneObject>("Hero", 0x200002, TypeRegistry::getInstance().findTypeByName("ZSTDOBJ"), std::move(geomInfo), std::move(instructions))};
	}

	// And then we may to map data to scene entities
	const auto &instructions = byteCode.getInstructions();
	Span ip{instructions};
	ASSERT_NO_THROW(SceneObjectPropertiesLoader::load(Span(sceneObjects), ip));

	// I think there no need to check properties, we will check only controllers here
	ASSERT_TRUE(sceneObjects[0]->getControllers().contains("Inventory"));

	const Type *inventory = TypeRegistry::getInstance().findTypeByShortName("Inventory");
	ASSERT_NE(inventory, nullptr);

	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getType(), inventory);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions().size(), 1);
	ASSERT_TRUE(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].getOpCode(), PRPOpCode::Int32);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].getOperand().trivial.i32, 42);
}

TEST_F(PRP_ComplexPack, DeclWithMultipleControllers)
{
	const uint8_t kByteCode[] = {
	    // Begin Object
	    (uint8_t) (PRPOpCode::BeginObject),

	    // BoundingBox represented as StringOrArray_E or StringOrArray_8E op-code
	    (uint8_t) (PRPOpCode::StringOrArray_E), 0x00, 0x00, 0x00, 0x00,// Operand is index of string in token table

	    // Matrix represented as BeginArray + 4 bytes capacity + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x09, 0x00, 0x00, 0x00,//9 entries

	    // First row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,

	    // Second row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    // Third row
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    (uint8_t) (PRPOpCode::EndArray),
	    // Vector represented as BeginArray + Int32 cap + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x03, 0x00, 0x00, 0x00,// 3 entries

	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x99, 0x99, 0x9A,//X = 1.2f
	    (uint8_t) (PRPOpCode::Float32), 0x41, 0x20, 0x00, 0x00,//Y = 10.f
	    (uint8_t) (PRPOpCode::Float32), 0xC0, 0xA8, 0x00, 0x00,//Z = -5.25f

	    (uint8_t) (PRPOpCode::EndArray),

	    // IsInactive - PRPOpCode::Bool
	    (uint8_t) (PRPOpCode::Bool), 0x1,

	    // PrimId - PRPOpCode::Int32
	    (uint8_t) (PRPOpCode::Int32), 0x05, 0x00, 0x00, 0x00,

	    // (ZSTDOBJ) - Invisible
	    (uint8_t) (PRPOpCode::Bool), 0x0,

	    // End Object
	    (uint8_t) (PRPOpCode::EndObject),

	    // Controllers (2)
	    (uint8_t) (PRPOpCode::Container), 0x02, 0x00, 0x00, 0x00,

	    // Controller[0] - Inventory
	    (uint8_t)(PRPOpCode::String), 0x01, 0x00, 0x00, 0x00, // Inventory (#1)
	    (uint8_t)(PRPOpCode::BeginObject),
	    (uint8_t)(PRPOpCode::Int32), 0x2A, 0x00, 0x00, 0x00, // CInventory::Money (0x2A => 42 coins)
	    (uint8_t)(PRPOpCode::EndObject),

	    // Controller[1] - Tie
	    (uint8_t)(PRPOpCode::String), 0x02, 0x00, 0x00, 0x00, // Tie (#2)
	    (uint8_t)(PRPOpCode::BeginObject),
	    (uint8_t)(PRPOpCode::Bool), 0x01, // bool  Tie::IsVisible  (0x01 => true)
	    (uint8_t)(PRPOpCode::Int32), 0xFF, 0x00, 0x00, 0x00, // int32 Tie::Multiplier (0xFF => 255)
	    (uint8_t)(PRPOpCode::EndObject),

	    // Children (no children)
	    (uint8_t) (PRPOpCode::Container), 0x00, 0x00, 0x00, 0x00,

	    // --- END OF STREAM ---
	    (uint8_t) (PRPOpCode::EndOfStream)};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("BOUNDING_DynamicAutoAssign"); // #0
	tokenTable.addToken("Inventory"); //#1
	tokenTable.addToken("Tie"); //#2

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	// Then create list of scene objects (just one object for this test)
	std::vector<SceneObject::Ptr> sceneObjects;

	{
		gamelib::gms::GMSGeomEntity geomInfo;
		SceneObject::Instructions instructions;

		sceneObjects = {
		    std::make_shared<SceneObject>("Hero", 0x200002, TypeRegistry::getInstance().findTypeByName("ZSTDOBJ"), std::move(geomInfo), std::move(instructions))};
	}

	// And then we may to map data to scene entities
	const auto &instructions = byteCode.getInstructions();
	Span ip{instructions};
	ASSERT_NO_THROW(SceneObjectPropertiesLoader::load(Span(sceneObjects), ip));

	// I think there no need to check properties, we will check only controllers here
	ASSERT_TRUE(sceneObjects[0]->getControllers().contains("Inventory"));
	ASSERT_TRUE(sceneObjects[0]->getControllers().contains("Tie"));

	const Type *inventory = TypeRegistry::getInstance().findTypeByShortName("Inventory");
	ASSERT_NE(inventory, nullptr);

	const Type *tie = TypeRegistry::getInstance().findTypeByShortName("Tie");
	ASSERT_NE(tie, nullptr);

	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getType(), inventory);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions().size(), 1);
	ASSERT_TRUE(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].getOpCode(), PRPOpCode::Int32);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Inventory"].getInstructions()[0].getOperand().trivial.i32, 42);

	ASSERT_EQ(sceneObjects[0]->getControllers()["Tie"].getType(), tie);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Tie"].getInstructions().size(), 2);
	ASSERT_TRUE(sceneObjects[0]->getControllers()["Tie"].getInstructions()[0].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["Tie"].getInstructions()[0].getOpCode(), PRPOpCode::Bool);
	ASSERT_TRUE(sceneObjects[0]->getControllers()["Tie"].getInstructions()[0].getOperand().trivial.b);
	ASSERT_TRUE(sceneObjects[0]->getControllers()["Tie"].getInstructions()[1].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["Tie"].getInstructions()[1].getOpCode(), PRPOpCode::Int32);
	ASSERT_EQ(sceneObjects[0]->getControllers()["Tie"].getInstructions()[1].getOperand().trivial.i32, 255);
}


TEST_F(PRP_ComplexPack, UnexposedTypeDecl)
{
	/**
	 * DISCLAIMER: We need to understand that "unexposed instructions" supported only for controller in complex object.
	 * 			   I mean that trivial ->map(...) will ignore that flag, this feature could be processed only in SceneObjectPropertiesLoader class
	 */
	const uint8_t kByteCode[] = {
	    // Begin Object
	    (uint8_t) (PRPOpCode::BeginObject),

	    // BoundingBox represented as StringOrArray_E or StringOrArray_8E op-code
	    (uint8_t) (PRPOpCode::StringOrArray_E), 0x02, 0x00, 0x00, 0x00,// Operand is index of string in token table

	    // Matrix represented as BeginArray + 4 bytes capacity + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x09, 0x00, 0x00, 0x00,//9 entries

	    // First row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,

	    // Second row
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    // Third row
	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x80, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,
	    (uint8_t) (PRPOpCode::Float32), 0x00, 0x00, 0x00, 0x00,

	    (uint8_t) (PRPOpCode::EndArray),
	    // Vector represented as BeginArray + Int32 cap + N entries + EndArray
	    (uint8_t) (PRPOpCode::Array), 0x03, 0x00, 0x00, 0x00,// 3 entries

	    (uint8_t) (PRPOpCode::Float32), 0x3F, 0x99, 0x99, 0x9A,//X = 1.2f
	    (uint8_t) (PRPOpCode::Float32), 0x41, 0x20, 0x00, 0x00,//Y = 10.f
	    (uint8_t) (PRPOpCode::Float32), 0xC0, 0xA8, 0x00, 0x00,//Z = -5.25f

	    (uint8_t) (PRPOpCode::EndArray),

	    // IsInactive - PRPOpCode::Bool
	    (uint8_t) (PRPOpCode::Bool), 0x1,

	    // PrimId - PRPOpCode::Int32
	    (uint8_t) (PRPOpCode::Int32), 0x05, 0x00, 0x00, 0x00,

	    // End Object
	    (uint8_t) (PRPOpCode::EndObject),

	    // Controllers [1]
	    (uint8_t)(PRPOpCode::Container), 0x01, 0x00, 0x00, 0x00,

	    // Controller #0 - ScriptC
	    (uint8_t)(PRPOpCode::String), 0x00, 0x00, 0x00, 0x00, // Str #0

	    (uint8_t)(PRPOpCode::BeginObject),

	    // ScriptName
	    (uint8_t)(PRPOpCode::String), 0x01, 0x00, 0x00, 0x00,  // Str #1

	    // StartHealth (!UNEXPOSED!)
	    (uint8_t)(PRPOpCode::Int32), 0x5F, 0x00, 0x00, 0x00, // StartHealth = 95

	    // IsEnemy (!UNEXPOSED!)
	    (uint8_t)(PRPOpCode::Bool), 0x00,

	    // IsScared (!UNEXPOSED!)
	    (uint8_t)(PRPOpCode::Bool), 0x01,

	    (uint8_t)(PRPOpCode::EndObject),

	    // Children (no chld)
	    (uint8_t)(PRPOpCode::Container), 0x00, 0x00, 0x00, 0x00,

	    // --- END OF STREAM ---
	    (uint8_t)(PRPOpCode::EndOfStream)
	};

	PRPHeader header(6u, false, false, true);

	PRPTokenTable tokenTable;
	tokenTable.addToken("ScriptC");
	tokenTable.addToken("AllLevels\\GenericNPC");
	tokenTable.addToken("BOUNDING_DynamicAutoAssign");

	PRPByteCode byteCode;
	ASSERT_TRUE(byteCode.parse(&kByteCode[0], sizeof(kByteCode), &header, &tokenTable));

	std::vector<SceneObject::Ptr> sceneObjects;

	{
		gamelib::gms::GMSGeomEntity geomInfo;
		SceneObject::Instructions instructions;

		sceneObjects = {
		    std::make_shared<SceneObject>("Hero", 0x0, TypeRegistry::getInstance().findTypeByName("ZGEOM"), std::move(geomInfo), std::move(instructions))};
	}

	// And then we may to map data to scene entities
	const auto &instructions = byteCode.getInstructions();
	Span ip{instructions};
	ASSERT_NO_THROW(SceneObjectPropertiesLoader::load(Span(sceneObjects), ip));

	const Type *scriptC = TypeRegistry::getInstance().findTypeByShortName("ScriptC");
	ASSERT_NE(scriptC, nullptr);

	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getType(), scriptC);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions().size(), 4);

	ASSERT_TRUE(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[0].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[0].getOpCode(), PRPOpCode::String);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[0].getOperand().str, "AllLevels\\GenericNPC");

	ASSERT_TRUE(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[1].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[1].getOpCode(), PRPOpCode::Int32);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[1].getOperand().trivial.i32, 95);

	ASSERT_TRUE(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[2].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[2].getOpCode(), PRPOpCode::Bool);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[2].getOperand().trivial.b, false);

	ASSERT_TRUE(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[3].isTrivialValue());
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[3].getOpCode(), PRPOpCode::Bool);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getInstructions()[3].getOperand().trivial.b, true);

	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getEntries().size, 1);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getEntries()[0].name, "ScriptName");
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getEntries()[0].instructions.iSize, 1);
	ASSERT_EQ(sceneObjects[0]->getControllers()["ScriptC"].getEntries()[0].instructions.iOffset, 0);
}

//TODO: Check children iteration (1 children, 2 children, two parallel objects with abstract ROOT)