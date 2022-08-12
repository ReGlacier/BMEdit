#include <GameLib/Scene/SceneObjectTypeNotFoundException.h>
#include <fmt/format.h>
#include <cassert>

using namespace gamelib::scene;


SceneObjectTypeNotFoundException::SceneObjectTypeNotFoundException(uint32_t objectIdx, uint32_t typeHash)
	: SceneObjectVisitorException(objectIdx, "")
{
	m_cachedWhat = fmt::format("Type hash 0x{:08X} not found in type database for object #{}", typeHash, objectIdx);
	m_errorDesc = "Type not found (by hash)";
}

SceneObjectTypeNotFoundException::SceneObjectTypeNotFoundException(uint32_t objectIdx, const std::string& typeName)
	: SceneObjectVisitorException(objectIdx, "")
{
	m_cachedWhat = fmt::format("Type '{}' not found in type database for object #{}", typeName, objectIdx);
	m_errorDesc = "Type not found (by name)";
}