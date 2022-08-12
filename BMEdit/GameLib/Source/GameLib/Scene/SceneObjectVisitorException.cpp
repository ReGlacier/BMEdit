#include <GameLib/Scene/SceneObjectVisitorException.h>
#include <fmt/format.h>
#include <utility>
#include <cassert>

using namespace gamelib::scene;


SceneObjectVisitorException::SceneObjectVisitorException(uint32_t objectId, std::string errorDescription)
	: m_errorDesc(std::move(errorDescription))
	, m_objectId(objectId)
{
	m_cachedWhat = fmt::format("Failed to visit scene object #{} (reason: {})", m_objectId, m_errorDesc);
}

const char* SceneObjectVisitorException::what() const
{
	return m_cachedWhat.data();
}