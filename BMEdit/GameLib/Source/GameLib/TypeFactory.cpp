#include <GameLib/TypeFactory.h>
#include <GameLib/TypeEnum.h>
#include <GameLib/TypeAlias.h>
#include <GameLib/TypeArray.h>
#include <GameLib/TypeContainer.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeBitfield.h>
#include <GameLib/TypeRawData.h>
#include <GameLib/NotImplemented.h>
#include <GameLib/PRP/PRPOpCode.h>


namespace gamelib
{
	std::unique_ptr<Type> TypeFactory::createFromJson(const nlohmann::json &json)
	{
		const std::string typeName = json["typename"].get<std::string>();
		const std::string kind = json["kind"].get<std::string>();
		const auto typeKind = fromString(kind);

		switch (typeKind) {
		case TypeKind::ALIAS: {
			if (!json.contains("alias")) {
				return nullptr;
			}

			auto &alias = json["alias"];

			if (!alias.is_string()) {
				return nullptr;
			}

			auto aliasTypeName = alias.get<std::string>();
			auto aliasTypeAsOpCode = prp::fromString(aliasTypeName);

			if (OPCODE_VALID(aliasTypeAsOpCode))
			{
				return std::make_unique<TypeAlias>(typeName, aliasTypeAsOpCode);
			}

			return std::make_unique<TypeAlias>(typeName, aliasTypeName);
		}
		case TypeKind::COMPLEX: {
			std::vector<ValueView> properties = {};
			std::string parentTypeName;
			bool allowToSkipUnexposedProperties = false;

			if (json.contains("parent"))
			{
				parentTypeName = json["parent"].get<std::string>();
			}

			if (json.contains("skip_unexposed_properties") && json["skip_unexposed_properties"].is_boolean())
			{
				allowToSkipUnexposedProperties = json["skip_unexposed_properties"].get<bool>();
			}

			if (json.contains("properties"))
			{
				auto &propertiesBlock = json["properties"];

				for (const auto &[_name, propertyInfo]: propertiesBlock.items())
				{
					if (!propertyInfo.contains("name") || !propertyInfo.contains("typename"))
					{
						return nullptr; // Invalid decl
					}

					const auto &propertyName = propertyInfo["name"];
					const auto &propertyTypeName = propertyInfo["typename"];

					if (!propertyName.is_string() || !propertyTypeName.is_string())
					{
						return nullptr;
					}

					// Maybe later I will support this
//					int offset = -1;
//					if (propertyInfo.contains("offset") && propertyInfo["offset"].is_number_integer())
//					{
//						offset = propertyInfo["offset"].get<int>();
//					}

					auto propertyAsPrpOpCode = prp::fromString(propertyTypeName);

					if (OPCODE_VALID(propertyAsPrpOpCode))
					{
						properties.emplace_back(propertyName.get<std::string>(), propertyAsPrpOpCode, nullptr);
					}
					else
					{
						properties.emplace_back(propertyName.get<std::string>(), propertyTypeName.get<std::string>(), nullptr);
					}
				}
			}

			if (parentTypeName.empty())
			{
				return std::make_unique<TypeComplex>(typeName, std::move(properties), nullptr, allowToSkipUnexposedProperties);
			}

			return std::make_unique<TypeComplex>(typeName, std::move(properties), parentTypeName, allowToSkipUnexposedProperties);
		}
		break;
		case TypeKind::ENUM: {
			if (!json.contains("enum")) {
				return nullptr;
			}

			auto &enumBlock = json["enum"];
			TypeEnum::Entries entries;

			for (const auto &[name, value]: enumBlock.items()) {
				if (!value.is_number_integer()) {
					return nullptr;
				}

				auto &possibleValue = entries.emplace_back();
				possibleValue.name = name;
				possibleValue.value = value.get<int>();
			}

			return std::make_unique<TypeEnum>(typeName, std::move(entries));
		}
		case TypeKind::ARRAY: {
			if (!json.contains("array")) {
				return nullptr;
			}

			auto &arrayBlock = json["array"];
			if (!arrayBlock.contains("expected_length") || !arrayBlock.contains("inner_opcode_type")) {
				return nullptr;
			}

			auto &expectedLength = arrayBlock["expected_length"];
			if (!expectedLength.is_number_integer()) {
				return nullptr;
			}

			auto &innerOpCodeType = arrayBlock["inner_opcode_type"];
			if (!innerOpCodeType.is_string()) {
				return nullptr;
			}

			auto innerOpCode = prp::fromString(innerOpCodeType.get<std::string>());
			if (!OPCODE_VALID(innerOpCode))
			{
				return nullptr;
			}

			//TODO: Support 'transform' field
			return std::make_unique<TypeArray>(typeName, innerOpCode, expectedLength.get<uint32_t>());
		}
		case TypeKind::BITFIELD: {
			if (!json.contains("bitfield")) {
				return nullptr;
			}

			auto &bitfield = json["bitfield"];
			if (!bitfield.contains("type") || !bitfield.contains("values")) {
				return nullptr;
			}

			auto &type = bitfield["type"];

			if (!type.is_string()) {
				return nullptr;
			}

			auto typeOpCode = prp::fromString(type.get<std::string>());
			if (!OPCODE_VALID(typeOpCode))
			{
				return nullptr;
			}

			auto &values = bitfield["values"];
			TypeBitfield::PossibleOptions options;
			for (const auto &[name, value]: values.items()) {
				if (!value.is_number_integer()) {
					return nullptr;
				}

				options[name] = value.get<uint32_t>();
			}

			return std::make_unique<TypeBitfield>(typeName, std::move(options));
		}
		case TypeKind::CONTAINER:
			return std::make_unique<TypeContainer>(typeName);
		case TypeKind::RAW_DATA:
			return std::make_unique<TypeRawData>(typeName);
		case TypeKind::NONE:
		default:
			return nullptr;
		}
	}
}