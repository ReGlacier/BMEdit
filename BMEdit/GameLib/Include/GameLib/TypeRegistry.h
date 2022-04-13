#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <GameLib/Type.h>

#include <nlohmann/json.hpp>


namespace gamelib
{
	class TypeRegistry
	{
		TypeRegistry();

	public:
		TypeRegistry(const TypeRegistry &) = delete;
		TypeRegistry(TypeRegistry &&) = delete;
		TypeRegistry &operator=(const TypeRegistry &) = delete;
		TypeRegistry &operator=(TypeRegistry &&) = delete;

		static TypeRegistry &getInstance();

		void reset();

		void registerTypes(
			std::vector<nlohmann::json> &&typeDeclarations,
			std::unordered_map<std::string, std::string> &&typeToHash);

		[[nodiscard]] const Type *findTypeByName(const std::string &typeName) const;
		[[nodiscard]] const Type *findTypeByHash(const std::string &hash) const;

		void forEachType(const std::function<void(const Type *)> &predicate);

	private:
		std::vector<std::unique_ptr<Type>> m_types;
		std::unordered_map<std::string, Type*> m_typesByHash;
		std::unordered_map<std::string, Type*> m_typesByName;
	};
}