#include <GameLib/Scene/SceneObject.h>


namespace gamelib::scene
{
	SceneObject::SceneObject() = default;

	SceneObject::SceneObject(std::string name, uint32_t typeId)
		: m_name(std::move(name)), m_typeId(typeId)
	{
	}

	const std::string &SceneObject::getName() const
	{
		return m_name;
	}

	uint32_t SceneObject::getTypeId() const
	{
		return m_typeId;
	}
}