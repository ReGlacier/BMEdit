#pragma once

#include <cstdint>

#include <string>
#include <memory>
#include <vector>

#include <GameLib/GMS/GMSGeomEntity.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/Span.h>
#include <GameLib/Type.h>
#include <glm/mat4x4.hpp>
#include <map>


namespace gamelib::scene
{
	class SceneObject
	{
	public:
		using Ptr = std::shared_ptr<SceneObject>;
		using Ref = std::weak_ptr<SceneObject>;
		using Instructions = std::vector<prp::PRPInstruction>;

		struct Controller
		{
			std::string name;
			Value properties;

			bool operator==(const std::string &controllerName) const;
			bool operator!=(const std::string &controllerName) const;
			bool operator==(const Controller &other) const;
			bool operator!=(const Controller &other) const;
		};

		using Controllers = std::vector<Controller>;
		using ControllerHandle = size_t;

		SceneObject();
		SceneObject(std::string name, uint32_t typeId, const Type *type, gms::GMSGeomEntity geomEntity, Instructions rawProperties);

		void setParent(const SceneObject::Ptr& parent);

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] uint32_t getTypeId() const;
		[[nodiscard]] const Type *getType() const;
		[[nodiscard]] const Instructions &getRawInstructions() const;
		[[nodiscard]] Instructions &getRawInstructions();
		[[nodiscard]] const Controllers &getControllers() const;
		[[nodiscard]] Controllers &getControllers();
		[[nodiscard]] const Value &getProperties() const;
		[[nodiscard]] Value &getProperties();
		[[nodiscard]] const gms::GMSGeomEntity &getGeomInfo() const;
		[[nodiscard]] gms::GMSGeomEntity &getGeomInfo();
		[[nodiscard]] const SceneObject::Ref &getParent() const;
		[[nodiscard]] const std::vector<SceneObject::Ref> &getChildren() const;
		[[nodiscard]] std::vector<SceneObject::Ref> &getChildren();

		/**
		 * @brief Calculate transform matrix for OpenGL and other render API buddies
		 * @note This function calculates matrix at runtime. So, it's not huge operation but avoid of frequency calling of this function, please.
		 * @return model matrix (scale, rotation and translate operations applied)
		 */
		[[nodiscard]] glm::mat4 getLocalTransform() const;

		/**
		 * @return Position object from ZGEOM or (0;0;0)
		 */
		[[nodiscard]] glm::vec3 getPosition() const;

		/**
		 * @note Unlike getLocalTransform this matrix created for DX9 renderer. Do not use this matrix without preparation!
		 * @note If object does not contains Matrix object this method will return identity matrix for OpenGL (specific of glm). To avoid of bugs use hasProperty before!
		 * @return Matrix object from ZGEOM or identity matrix
		 */
		[[nodiscard]] glm::mat3 getOriginalTransform() const;

		/**
		 * @brief Calculate world transform of object
		 * @note This method iterates over all parents and multiply all matrices into one combined matrix. It may take a while so use external caches (because SceneObject does not make any caches itself)
		 * @return World model matrix of object
		 */
		[[nodiscard]] glm::mat4 getWorldTransform() const;

	private:
		std::string m_name {}; ///< Name of geom
		uint32_t m_typeId { 0u }; ///< Type ID of geom
		const Type *m_type { nullptr }; ///< Type of geom
		gms::GMSGeomEntity m_geom {}; ///< Base geom info
		SceneObject::Ref m_parent {}; ///< Parent geom
		std::vector<SceneObject::Ref> m_children {}; ///< Children geoms
		Instructions m_rawProperties {}; ///< Property instructions
		Controllers m_controllers; ///< Controllers
		Value m_properties;
	};
}