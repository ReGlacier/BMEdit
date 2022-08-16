#include <GameLib/Scene/SceneObject.h>

#include <utility>


namespace gamelib::scene
{
	SceneObject::SceneObject() = default;

	SceneObject::SceneObject(std::string name, uint32_t typeId, const Type *type, gms::GMSGeomEntity geomEntity, Instructions rawProperties)
		: m_name(std::move(name))
		, m_typeId(typeId)
		, m_type(type)
		, m_geom(std::move(geomEntity))
		, m_rawProperties(std::move(rawProperties))
	{
	}

	void SceneObject::setParent(const SceneObject::Ptr &parent)
	{
		m_parent = parent;
	}

	const std::string &SceneObject::getName() const
	{
		return m_name;
	}

	uint32_t SceneObject::getTypeId() const
	{
		return m_typeId;
	}

	const Type *SceneObject::getType() const
	{
		return m_type;
	}

	const SceneObject::Instructions &SceneObject::getRawInstructions() const
	{
		return m_rawProperties;
	}

	SceneObject::Instructions &SceneObject::getRawInstructions()
	{
		return m_rawProperties;
	}

	const SceneObject::Controllers &SceneObject::getControllers() const
	{
		return m_controllers;
	}

	SceneObject::Controllers &SceneObject::getControllers()
	{
		return m_controllers;
	}

	const Value &SceneObject::getProperties() const
	{
		return m_properties;
	}

	Value &SceneObject::getProperties()
	{
		return m_properties;
	}

	const gms::GMSGeomEntity &SceneObject::getGeomInfo() const
	{
		return m_geom;
	}

	gms::GMSGeomEntity &SceneObject::getGeomInfo()
	{
		return m_geom;
	}

	const SceneObject::Ref &SceneObject::getParent() const
	{
		return m_parent;
	}

	const std::vector<SceneObject::Ref> &SceneObject::getChildren() const
	{
		return m_children;
	}

	std::vector<SceneObject::Ref> &SceneObject::getChildren()
	{
		return m_children;
	}
}