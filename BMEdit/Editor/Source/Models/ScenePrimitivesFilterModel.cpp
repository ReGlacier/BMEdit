#include <Models/ScenePrimitivesFilterModel.h>
#include <Types/QCustomRoles.h>
#include <Types/QPrimType.h>


using namespace models;



int ScenePrimitivesFilterModel::getFilterMask() const
{
	return m_filterMask;
}

void ScenePrimitivesFilterModel::setAllowAll()
{
	if (m_filterMask != ScenePrimitivesFilterEntry::Allow_All)
	{
		m_filterMask = ScenePrimitivesFilterEntry::Allow_All;
		invalidateRowsFilter();
	}
}

void ScenePrimitivesFilterModel::setAllowNone()
{
	if (m_filterMask != ScenePrimitivesFilterEntry::Allow_None)
	{
		m_filterMask = ScenePrimitivesFilterEntry::Allow_None;
		invalidateRowsFilter();
	}
}

void ScenePrimitivesFilterModel::addFilterEntry(ScenePrimitivesFilterEntry entry)
{
	if (!(m_filterMask & entry))
	{
		m_filterMask |= entry;
		invalidateRowsFilter();
	}
}

void ScenePrimitivesFilterModel::removeFilterEntry(ScenePrimitivesFilterEntry entry)
{
	if (m_filterMask & entry)
	{
		m_filterMask &= ~entry;
		invalidateRowsFilter();
	}
}

bool ScenePrimitivesFilterModel::isVertexFormatAllowed(gamelib::prm::PRMVertexBufferFormat vertexBufferFormat)
{
	return m_allowedVertexFormats.contains(vertexBufferFormat);
}

void ScenePrimitivesFilterModel::setVertexFormatAllowed(gamelib::prm::PRMVertexBufferFormat vertexBufferFormat, bool isAllowed)
{
	if (isAllowed)
	{
		auto [iter, isInserted] = m_allowedVertexFormats.emplace(vertexBufferFormat);
		if (isInserted)
		{
			invalidateRowsFilter();
		}
	}
	else
	{
		if (auto it = m_allowedVertexFormats.find(vertexBufferFormat); it != m_allowedVertexFormats.end())
		{
			m_allowedVertexFormats.erase(vertexBufferFormat);
			invalidateRowsFilter();
		}
	}
}

void ScenePrimitivesFilterModel::resetToDefaults()
{
	setAllowAll();
	m_allowedVertexFormats = { g_DefaultVertexFormats.begin(), g_DefaultVertexFormats.end() };
}

bool ScenePrimitivesFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	// Pre-opt
	if (m_filterMask == ScenePrimitivesFilterEntry::Allow_None)
	{
		return false;
	}

	if (m_filterMask == ScenePrimitivesFilterEntry::Allow_All)
	{
		return true;
	}

	QModelIndex entryIndex = sourceModel()->index(sourceRow, 0, sourceParent);

	const auto value = sourceModel()->data(entryIndex, types::kChunkKindRole).value<gamelib::prm::PRMChunkRecognizedKind>();

	switch (value)
	{
		case gamelib::prm::PRMChunkRecognizedKind::CRK_ZERO_CHUNK:         return m_filterMask & ScenePrimitivesFilterEntry::FilterAllow_Zero;
		case gamelib::prm::PRMChunkRecognizedKind::CRK_INDEX_BUFFER:       return m_filterMask & ScenePrimitivesFilterEntry::FilterAllow_Index;
		case gamelib::prm::PRMChunkRecognizedKind::CRK_VERTEX_BUFFER:
	    {
		    if (m_filterMask & ScenePrimitivesFilterEntry::FilterAllow_Vertex)
		    {
			    if (g_DefaultVertexFormats.size() != m_allowedVertexFormats.size())
			    {
				    auto format = sourceModel()->data(entryIndex, types::kChunkVertexFormatRole).value<gamelib::prm::PRMVertexBufferFormat>();
				    if (m_allowedVertexFormats.contains(format))
				    {
					    return true;
				    }
			    }
			    else
			    {
				    return true;
			    }
		    }

		    return false;
	    }
		case gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER: return m_filterMask & ScenePrimitivesFilterEntry::FilterAllow_Description;
		case gamelib::prm::PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER:     return m_filterMask & ScenePrimitivesFilterEntry::FilterAllow_Unknown;
	}

	return false;
}