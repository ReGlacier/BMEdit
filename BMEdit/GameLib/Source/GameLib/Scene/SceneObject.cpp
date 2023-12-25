#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRPObjectExtractor.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <GameLib/TypeComplex.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

	bool SceneObject::isInheritedOf(const std::string& baseType) const
	{
		const auto* type = getType();

		if (!type) return false;

		while (type)
		{
			if (type->getName() == baseType)
				return true;

			if (type->getKind() != TypeKind::COMPLEX)
				return false;

			type = static_cast<const TypeComplex*>(type)->getParent(); // NOLINT(*-pro-type-static-cast-downcast)
		}

		return false;
	}

	glm::mat4 SceneObject::getLocalTransform() const
	{
		const auto vPosition  = getPosition();
		const auto mMatrix    = getOriginalTransform();

		glm::mat4 mResult = glm::mat4(1.f);
		glm::mat3 mMatrixT = glm::transpose(mMatrix);

		const float* pSrcMatrix = glm::value_ptr(mMatrix);
		float* pDstMatrix = glm::value_ptr(mResult);

		// Result of reverse engineering of MatPosToMatrix (or Transform3x3To4x4Matrix from PC version (sub_489740))
		pDstMatrix[0]  = pSrcMatrix[6];
		pDstMatrix[1]  = pSrcMatrix[7];
		pDstMatrix[2]  = pSrcMatrix[8];
		pDstMatrix[3]  = 0.0f;
		pDstMatrix[4]  = pSrcMatrix[3];
		pDstMatrix[5]  = pSrcMatrix[4];
		pDstMatrix[6]  = pSrcMatrix[5];
		pDstMatrix[7]  = 0.0f;
		pDstMatrix[8]  = pSrcMatrix[0];
		pDstMatrix[9]  = pSrcMatrix[1];
		pDstMatrix[10] = pSrcMatrix[2];
		pDstMatrix[11] = 0.0f;
		pDstMatrix[12] = vPosition.x;
		pDstMatrix[13] = vPosition.y;
		pDstMatrix[14] = vPosition.z;
		pDstMatrix[15] = 1.0f;

		return mResult;
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

	void SceneObject::visitChildren(const std::function<EVisitResult(const gamelib::scene::SceneObject::Ptr&)>& pred) const
	{
		if (!pred)
			return;

		internalVisitChildObjects(pred);
	}

	SceneObject::EVisitResult SceneObject::internalVisitChildObjects(const std::function<EVisitResult(const gamelib::scene::SceneObject::Ptr &)>& pred) const
	{
		for (const auto rChild : getChildren())
		{
			if (auto pChild = rChild.lock())
			{
				const auto predRes = pred(pChild);

				switch (predRes)
				{
					case EVisitResult::VR_CONTINUE:
					{
						auto predResInner = pChild->internalVisitChildObjects(pred);
						if (predResInner == EVisitResult::VR_STOP_ALL)
							return predResInner;
					}
					break;

					case EVisitResult::VR_STOP_ALL: return EVisitResult::VR_STOP_ALL;
					case EVisitResult::VR_NEXT: continue;
				}
			}
		}

		return EVisitResult::VR_CONTINUE;
	}
}