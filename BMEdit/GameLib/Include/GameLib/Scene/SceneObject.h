#pragma once

#include <cstdint>

#include <string>
#include <memory>
#include <vector>

#include <GameLib/GMS/GMSGeomEntity.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/Span.h>
#include <GameLib/Type.h>
#include <map>


namespace gamelib::scene
{
	class SceneObject
	{
	public:
		using Ptr = std::shared_ptr<SceneObject>;
		using Ref = std::weak_ptr<SceneObject>;
		using Instructions = std::vector<prp::PRPInstruction>;
		using Controllers = std::map<std::string, Value>;

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

	private:
		std::string m_name {}; ///< Name of geom
		uint32_t m_typeId { 0u }; ///< Type ID of geom
		const Type *m_type { nullptr }; ///< Type of geom
		gms::GMSGeomEntity m_geom {}; ///< Base geom info
		SceneObject::Ref m_parent {}; ///< Parent geom
		std::vector<SceneObject::Ref> m_children {}; ///< Children geoms
		Instructions m_rawProperties {}; ///< Property instructions
		std::map<std::string, Value> m_controllers; ///< Controllers
		Value m_properties;
	};
}