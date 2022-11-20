#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeFactory.h>
#include <GameLib/TypeAlias.h>
#include <GameLib/TypeArray.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeNotFoundException.h>

#include <sstream>


namespace gamelib
{
	TypeRegistry::TypeRegistry() = default;

	TypeRegistry &TypeRegistry::getInstance()
	{
		static TypeRegistry g_typeRegistryInstance;
		return g_typeRegistryInstance;
	}

	void TypeRegistry::reset()
	{
		m_typesByHash.clear();
		m_typesByName.clear();
		m_types.clear();
	}

	void TypeRegistry::registerTypes(std::vector<nlohmann::json> &&typeDeclarations, std::unordered_map<std::string, std::string> &&typeToHash)
	{
		reset();

		m_types.reserve(typeDeclarations.size());

		for (const auto &jsonDeclaration: typeDeclarations)
		{
			if (!jsonDeclaration.contains("typename") || !jsonDeclaration.contains("kind"))
			{
				throw std::runtime_error("Invalid type declaration!");
			}

			const std::string typeName = jsonDeclaration["typename"].get<std::string>();

			// Produce type by kind
			auto typeInstance = TypeFactory::createFromJson(jsonDeclaration);
			if (!typeInstance)
			{
				throw std::runtime_error("Failed to create type '" + typeName + "'.");
			}

			auto typePtr = typeInstance.get();
			m_types.emplace_back(std::move(typeInstance));

			m_typesByName[typeName] = typePtr;
			if (typeToHash.contains(typeName))
			{
				m_typesByHash[typeToHash[typeName]] = typePtr;
			}
		}

		linkTypes();
	}

	const Type *TypeRegistry::findTypeByName(const std::string &typeName) const
	{
		auto it = m_typesByName.find(typeName);
		if (it == m_typesByName.end())
		{
			return nullptr;
		}

		return it->second;
	}

	const Type *TypeRegistry::findTypeByHash(const std::string &hash) const
	{
		auto it = m_typesByHash.find(hash);
		if (it == m_typesByHash.end())
		{
			return nullptr;
		}

		return it->second;
	}

	const Type *TypeRegistry::findTypeByHash(std::size_t typeId) const
	{
		std::stringstream stringStream;
		stringStream << "0x" << std::hex << std::uppercase << typeId;
		auto str = stringStream.str();

		return findTypeByHash(str);
	}

	const Type *TypeRegistry::findTypeByShortName(const std::string &requestedTypeName) const
	{
		for (const auto& type: m_types)
		{
			const std::string &typeName = type->getName();
			if (typeName.empty())
				continue;

			const bool hasClassPrefix = typeName[0] == 'Z' || typeName[0] == 'C';

			// >>> IOI Hacks starts here <<<
			// #0 : trivial equality
			if (typeName == requestedTypeName)
				return type.get();

			// #1 : IOI G1 codegen remove Z & C prefix from typename
			if (hasClassPrefix && ((typeName.length() - 1) == requestedTypeName.length() && std::equal(typeName.begin() + 1, typeName.end(), requestedTypeName.begin())))
				return type.get();

			// #2 : IOI G1 codegen remove Event postfix from typename too
			if (hasClassPrefix && std::string("Z").append(requestedTypeName).append("Event") == typeName)
				return type.get();

			// #3 : IOI G1 codegen for Hitman Blood Money (HM3) removing HM3 prefix
			if (hasClassPrefix && std::string("ZHM3").append(requestedTypeName) == typeName)
				return type.get();

			// #4 : IOI G1 codegen for Hitman Blood Money (HM3) removing HM3 prefix w/o Z
			if (std::string("HM3").append(requestedTypeName) == typeName)
				return type.get();

			//NOTE: Maybe we should optimize this place
		}

		return nullptr;
	}

	void TypeRegistry::forEachType(const std::function<void(const Type *)> &predicate)
	{
		if (!predicate)
		{
			return;
		}

		for (const auto& type: m_types)
		{
			predicate(type.get());
		}
	}

	void TypeRegistry::linkTypes()
	{
		// Resolve links
		for (const auto& type: m_types)
		{
			if (auto kind = type->getKind(); kind == TypeKind::ALIAS)
			{
				auto alias = reinterpret_cast<TypeAlias *>(type.get());
				if (auto referenceStrPtr = std::get_if<std::string>(&alias->m_resultTypeInfo); referenceStrPtr != nullptr)
				{
					auto typePtr = findTypeByName(*referenceStrPtr);
					if (!typePtr)
					{
						throw TypeNotFoundException("Failed to resolve link to type '" + *referenceStrPtr + "' from type '" + alias->getName() + "'!");
					}

					alias->m_resultTypeInfo = reinterpret_cast<const Type *>(typePtr);
				}
			}
			else if (kind == TypeKind::COMPLEX)
			{
				// So, here we need to iterate over all properties and parent class
				auto complex = reinterpret_cast<TypeComplex *>(type.get());

				// Take parent
				if (auto parentTypeNamePtr = std::get_if<std::string>(&complex->m_parent); parentTypeNamePtr != nullptr)
				{
					auto newParentType = findTypeByName(*parentTypeNamePtr);
					if (!newParentType)
					{
						throw std::runtime_error("Failed to resolve link to parent type '" + *parentTypeNamePtr + "' from type '" + complex->getName() + "'!");
					}

					complex->m_parent = newParentType;
				}

				// Resolve links from views
				for (auto &property: complex->m_instructionViews)
				{
					if (auto propertyTypeNamePtr = std::get_if<std::string>(&property.m_type); propertyTypeNamePtr != nullptr)
					{
						auto newPropertyType = findTypeByName(*propertyTypeNamePtr);

						if (!newPropertyType)
						{
							throw TypeNotFoundException("Failed to resolve link to property type '" + *propertyTypeNamePtr + "' from type '" + complex->getName() + "'!");
						}

						property.m_type = newPropertyType;
					}

					if (property.m_ownerType == nullptr)
					{
						property.m_ownerType = complex; //setup owner if not inited
					}
				}
			}
			else if (kind == TypeKind::ARRAY)
			{
				// Here we need to resolve 'owner' alias if it not inited
				auto array = reinterpret_cast<TypeArray *>(type.get());
				for (auto &view: array->m_valueViews)
				{
					if (view.m_ownerType == nullptr)
					{
						view.m_ownerType = array;
					}
				}
			}
		}
	}

	void TypeRegistry::addHashAssociation(std::size_t hash, const std::string &typeName)
	{
		if (auto typePtr = findTypeByName(typeName))
		{
			std::stringstream stringStream;
			stringStream << "0x" << std::hex << std::uppercase << hash;
			auto str = stringStream.str();

			m_typesByHash[str] = const_cast<Type*>(typePtr);
		}
	}

	bool TypeRegistry::canCastImpl(const gamelib::Type *pSrc, const gamelib::Type *pDst) const // NOLINT(misc-no-recursion)
	{
		if (!pSrc || !pDst || pSrc->getKind() != TypeKind::COMPLEX || pDst->getKind() != TypeKind::COMPLEX)
		{
			return false;
		}

		if (pSrc->getName() == pDst->getName())
		{
			return true;
		}

		return canCastImpl(reinterpret_cast<const TypeComplex*>(pSrc)->getParent(), pDst);
	}
}