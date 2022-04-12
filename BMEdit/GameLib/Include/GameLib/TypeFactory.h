#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include <GameLib/Type.h>


namespace gamelib
{
	class TypeFactory
	{
	public:
		static std::unique_ptr<Type> createFromJson(const nlohmann::json &json);
	};
}