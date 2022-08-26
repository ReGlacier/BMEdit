#include <GameLib/Scene/SceneObjectPropertiesLoader.h>
#include <GameLib/Scene/SceneObjectTypeNotFoundException.h>
#include <GameLib/Scene/SceneObjectVisitorException.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeAlias.h>

#include <fmt/format.h>

#define NEXT_IP nextInstruction();
#define NEXT_OBJECT nextObject();

namespace gamelib::scene
{
	using gamelib::prp::PRPOpCode;
	using gamelib::prp::PRPInstruction;
	using gamelib::scene::SceneObjectPropertiesLoader;

	struct InternalContext
	{
		int32_t objectIdx = 0;
		Span<SceneObject::Ptr> objects;
		Span<PRPInstruction> ip;

		void visitImpl(const SceneObject::Ptr& parent = nullptr);

		void nextObject()
		{
			assert(!objects.empty());

			++objectIdx;
		}

		void nextInstruction()
		{
			++ip;
		}

		[[nodiscard]] SceneObject::Ptr getCurrentObject() const
		{
			return objects ? objects[objectIdx] : nullptr;
		}
	};

	void SceneObjectPropertiesLoader::load(Span<SceneObject::Ptr> objects, Span<PRPInstruction> instructions)
	{
		if (!objects || !instructions)
			return;

		InternalContext ctx;
		ctx.ip = instructions;
		ctx.objects = objects;
		ctx.objectIdx = 0;
		ctx.visitImpl();
	}

	void InternalContext::visitImpl(const SceneObject::Ptr& parent) // NOLINT(misc-no-recursion)
	{
		const auto& currentObject = getCurrentObject();

//		printf("PROCESS UNIT #%d '%s' (of type %.08X)\n", objectIdx, currentObject->getName().data(), currentObject->getTypeId());
//		fflush(stdout);

		/**
		 * Generic object declaration rules:
		 * 		Here we working with Glacier (R) object definition format. This format includes three sections:
		 * 		1) Object properties:
		 * 			[BeginObject|BeginNamedObject]
		 * 				(Properties)
		 * 			[EndObject]
		 * 		2) Controllers
		 * 			[Container - controllers count]
		 * 				[String - name]
		 * 				[BeginObject|BeginNamedObject]
		 * 					(Properties...)
		 * 				[EndObject]
		 * 		3) Children
		 * 			[Container - children count]
		 * 				[BeginObject|BeginNamedObject]
		 * 				<ZGEOM>
		 * 				[EndObject] ?
		 */

		/// ------------ STAGE 1: PROPERTIES ------------
		if (ip[0].getOpCode() != PRPOpCode::BeginObject && ip[0].getOpCode() != PRPOpCode::BeginNamedObject)
		{
			throw SceneObjectVisitorException(objectIdx, "Invalid object definition (expected BeginObject/BeginNamedObject)");
		}

		NEXT_IP

		// Check type
		const Type* objectType = TypeRegistry::getInstance().findTypeByHash(currentObject->getTypeId());
		if (!objectType)
		{
			throw SceneObjectTypeNotFoundException(objectIdx, currentObject->getTypeId());
		}

		// Read properties
		{
			const auto& [vRes, _newInstructions] = objectType->verify(ip);
			if (!vRes)
			{
				throw SceneObjectVisitorException(objectIdx, "Invalid instructions set (verification failed)");
			}

			const auto& [value, newIP] = objectType->map(ip);

			if (!value.has_value())
			{
				throw SceneObjectVisitorException(objectIdx, "Invalid instructions set (verification failed) [2]");
			}

			currentObject->getProperties() = *value;
			ip = newIP; // Assign new ip
		}

		// Check that object ends with EndObject opcode
		if (ip[0].getOpCode() != PRPOpCode::EndObject)
		{
			throw SceneObjectVisitorException(objectIdx, "Object decl must ends with EndObject");
		}

		NEXT_IP

		/// ------------ STAGE 2: CONTROLLERS ------------
		if (ip[0].getOpCode() != PRPOpCode::Container && ip[0].getOpCode() == PRPOpCode::NamedContainer)
		{
			throw SceneObjectVisitorException(objectIdx, "Invalid object definition (Expected Container/NamedContainer)");
		}

		const auto controllersCount = ip[0].getOperand().trivial.i32;

		NEXT_IP

		if (controllersCount > 0)
		{
			for (int32_t controllerIdx = 0; controllerIdx < controllersCount; ++controllerIdx)
			{
				if (ip[0].getOpCode() != PRPOpCode::String)
				{
					throw SceneObjectVisitorException(objectIdx, "Invalid controller definition (Expected String)");
				}

				const std::string& controllerName = ip[0].getOperand().str;

				NEXT_IP

				if (ip[0].getOpCode() != PRPOpCode::BeginObject && ip[0].getOpCode() != PRPOpCode::BeginNamedObject)
				{
					throw SceneObjectVisitorException(objectIdx, "Invalid controller definition (Expected BeginObject/BeginNamedObject)");
				}

				NEXT_IP

				// Find type
				const Type* controllerType = TypeRegistry::getInstance().findTypeByShortName(controllerName);
				if (!controllerType)
				{
					throw SceneObjectTypeNotFoundException(objectIdx, controllerName);
				}

				if (controllerType->getKind() != TypeKind::COMPLEX)
				{
					bool bIsNotAComplexType = true;

					if (controllerType->getKind() == TypeKind::ALIAS)
					{
						if (auto finalType = reinterpret_cast<const TypeAlias*>(controllerType)->getFinalType())
						{
							bIsNotAComplexType = finalType->getKind() != TypeKind::COMPLEX;
						}
					}

					if (bIsNotAComplexType)
					{
						// Only complex types are allowed to be controllers
						throw SceneObjectVisitorException(objectIdx, fmt::format("Type '{}' not allowed to be controller because it's not COMPLEX", controllerType->getName()));
					}
				}

				// Map controller properties
				const auto& [controllerMapResult, nextIP] = controllerType->map(ip);

				if (!controllerMapResult.has_value())
				{
					throw SceneObjectVisitorException(objectIdx, fmt::format("Failed to map controller '{}'", controllerName));
				}

				ip = nextIP;

				currentObject->getControllers()[controllerName] = controllerMapResult.value();

				if (ip[0].getOpCode() != PRPOpCode::EndObject && reinterpret_cast<const TypeComplex*>(controllerType)->areUnexposedInstructionsAllowed())
				{
					// Here we need to extract all instructions until 'EndObject'
					Span<PRPInstruction> begin = ip, end = ip;
					int64_t endOffset = 0;

					// Find nearest 'EndObject' instruction and instruction before it will be our last instruction
					while (!end.empty() && end[0].getOpCode() != PRPOpCode::EndObject)
					{
						++end;
						++endOffset;
					}

					if (end.empty())
					{
						throw SceneObjectVisitorException(objectIdx, fmt::format("Invalid controller definition: We have controller '{}' with unexposed instructions and without EndObject instruction!", controllerName));
					}

					auto unexposedInstructions = begin.slice(0, endOffset).as<std::vector<PRPInstruction>>();

					// NOTE: maybe we should think about how to avoid copies here?
					auto& orgInstructionsRef = currentObject->getControllers().at(controllerName).getInstructions();
					orgInstructionsRef.insert(orgInstructionsRef.end(), unexposedInstructions.begin(), unexposedInstructions.end());

					ip = ip.slice(endOffset, ip.size() - endOffset);
				}

				if (ip[0].getOpCode() != PRPOpCode::EndObject)
				{
					throw SceneObjectVisitorException(objectIdx, "Invalid controller definition (Expected EndObject)");
				}

				NEXT_IP
			}
		}

		if (parent)
		{
			currentObject->setParent(parent);
		}

		NEXT_OBJECT

		/// ------------ STAGE 3: CHILDREN ------------
		/*
		 * 		3) Children
		 * 			[Container - children count]
		 * 				[BeginObject|BeginNamedObject]
		 * 				<ZGEOM>
		 * 				[EndObject] ?
		 */
		if (ip[0].getOpCode() != PRPOpCode::Container && ip[0].getOpCode() != PRPOpCode::NamedContainer)
		{
			throw SceneObjectVisitorException(objectIdx, "Invalid controller definition (Expected Container with children geoms)");
		}

		const int32_t childrenCount = ip[0].getOperand().trivial.i32;
		NEXT_IP

		if (childrenCount > 0)
		{
			if (ip[0].getOpCode() != PRPOpCode::BeginObject && ip[0].getOpCode() != PRPOpCode::BeginNamedObject)
			{
				throw SceneObjectVisitorException(objectIdx, "Invalid children definition (expected BeginObject/BeginNamedObject)");
			}

			for (int32_t geomIdx = 0; geomIdx < childrenCount; ++geomIdx)
			{
				currentObject->getChildren().push_back(getCurrentObject());
				visitImpl(currentObject);
			}
		}
	}
}

#undef NEXT_IP
#undef NEXT_OBJECT