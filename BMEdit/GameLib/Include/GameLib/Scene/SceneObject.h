#pragma once

#include <string>
#include <cstdint>


namespace gamelib::scene
{
	class SceneObject
	{
	public:
		SceneObject();
		SceneObject(std::string name, uint32_t typeId);

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] uint32_t getTypeId() const;
	private:
		std::string m_name;
		uint32_t m_typeId;
	};
}