#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <GameLib/Type.h>
#include <GameLib/StringLiteral.h>

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
		[[nodiscard]] const Type *findTypeByHash(std::size_t hash) const;
		[[nodiscard]] const Type *findTypeByShortName(const std::string &typeName) const;

		void forEachType(const std::function<void(const Type *)> &predicate);

		void linkTypes();
		void addHashAssociation(std::size_t hash, const std::string &typeName);

		template <typename T>
		T* registerType(std::unique_ptr<T>&& constructedType) requires (std::is_base_of_v<Type, T>)
		{
			if (!constructedType)
				return nullptr;

			T* ptr = constructedType.get();

			const auto& [_iter, res] = m_typesByName.try_emplace(ptr->getName(), ptr);
			if (!res)
				return nullptr;

			m_types.emplace_back(std::move(constructedType));

			return ptr;
		}

		template <StringLiteral CastToName>
		static bool canCast(const Type* tsrc)
		{
			if (!tsrc || tsrc->getKind() != TypeKind::COMPLEX)
			{
				return false;
			}

			auto str = std::string(CastToName.Value);
			const auto& instance = TypeRegistry::getInstance();

			auto finalType = instance.findTypeByName(str);
			if (!finalType)
			{
				return false;
			}

			return instance.canCastImpl(tsrc, finalType);
		}

	private:
		bool canCastImpl(const Type* pSrc, const Type* pDst) const;

	private:
		std::vector<std::unique_ptr<Type>> m_types;
		std::unordered_map<std::string, Type*> m_typesByHash;
		std::unordered_map<std::string, Type*> m_typesByName;
	};
}