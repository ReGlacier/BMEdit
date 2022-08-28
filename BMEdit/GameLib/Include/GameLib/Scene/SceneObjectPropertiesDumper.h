#pragma once

#include <cstdint>
#include <vector>
#include <memory>


namespace gamelib
{
	class Level;
}

namespace gamelib::scene
{
	class SceneObject;

	class SceneObjectPropertiesDumper
	{
	public:
		SceneObjectPropertiesDumper();
		~SceneObjectPropertiesDumper();

		void dump(const Level *level, std::vector<uint8_t> *outBuffer);

	private:
		void visitSceneObject(const SceneObject *sceneObject);

	private:
		struct DumperContext;
		std::unique_ptr<DumperContext> m_localContext { nullptr };
	};
}