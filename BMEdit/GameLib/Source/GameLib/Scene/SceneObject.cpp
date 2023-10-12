#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <utility>


namespace gamelib::scene
{
	bool SceneObject::Controller::operator==(const std::string &controllerName) const
	{
		return name == controllerName;
	}

	bool SceneObject::Controller::operator!=(const std::string &controllerName) const
	{
		return !operator==(controllerName);
	}

	bool SceneObject::Controller::operator==(const SceneObject::Controller &other) const
	{
		return name == other.name && properties == other.properties;
	}

	bool SceneObject::Controller::operator!=(const SceneObject::Controller &other) const
	{
		return !operator==(other);
	}

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

	glm::mat4 SceneObject::getLocalTransform() const
	{
		const auto vPosition  = getProperties().getObject<glm::vec3>("Position", glm::vec3(0.f));
		const auto mMatrix    = getProperties().getObject<glm::mat3>("Matrix", glm::mat3(1.f));

		// Extract matrix from properties
		glm::mat4 mTransform = glm::mat4(1.f);

		/**
		 * Convert from DX9 to OpenGL
		 *
		 *        | m00 m10 m20 |
		 * mSrc = | m01 m11 m21 |
		 *        | m02 m12 m22 |
		 *
		 *        | m02 m01 m00 |
		 * mDst = | m12 m11 m21 |
		 *        | m22 m21 m20 |
		 */
		mTransform[0][0] = mMatrix[0][2];
		mTransform[1][0] = mMatrix[0][1];
		mTransform[2][0] = mMatrix[0][0];

		mTransform[0][1] = mMatrix[1][2];
		mTransform[1][1] = mMatrix[1][1];
		mTransform[2][1] = mMatrix[2][1];

		mTransform[0][2] = mMatrix[2][2];
		mTransform[1][2] = mMatrix[2][1];
		mTransform[2][2] = mMatrix[2][0];

		mTransform[3][3] = 1.f;

		glm::mat4 mTranslate = glm::translate(glm::mat4(1.f), vPosition);
		glm::mat4 mModelMatrix = mTranslate * mTransform;

		return mModelMatrix;
	}

	glm::vec3 SceneObject::getPosition() const
	{
		return getProperties().getObject<glm::vec3>("Position", glm::vec3(0.f));
	}

	glm::mat3 SceneObject::getOriginalTransform() const
	{
		return getProperties().getObject<glm::mat3>("Matrix", glm::mat3(1.f));
	}

	glm::mat4 SceneObject::getWorldTransform() const
	{
		const SceneObject* current = this;
		glm::mat4 mWorldMatrix = glm::mat4(1.f);

		while (current)
		{
			mWorldMatrix = mWorldMatrix * current->getLocalTransform();

			current = current->getParent().expired() ? nullptr : current->getParent().lock().get();
		}

		return mWorldMatrix;
	}
}