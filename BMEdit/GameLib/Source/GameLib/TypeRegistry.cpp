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

					alias->m_resultTypeInfo = reinterpret_cast<const Type *>(alias);
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
}