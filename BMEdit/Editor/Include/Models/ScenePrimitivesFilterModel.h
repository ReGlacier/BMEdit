#pragma once

#include <QSortFilterProxyModel>
#include <GameLib/PRM/PRMVertexFormat.h>
#include <set>


namespace models
{
	constexpr std::array<gamelib::prm::PRMVertexBufferFormat, 4> g_DefaultVertexFormats = {
	    gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_10,
	    gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_24,
	    gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_28,
	    gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_34
	};

	enum ScenePrimitivesFilterEntry : int
	{
		FilterAllow_Zero        = 1 << 0,
		FilterAllow_Unknown     = 1 << 1,
		FilterAllow_Description = 1 << 2,
		FilterAllow_Index       = 1 << 3,
		FilterAllow_Vertex      = 1 << 4,

		// Common values
		Allow_None = 0,
		Allow_All = FilterAllow_Zero | FilterAllow_Unknown | FilterAllow_Description | FilterAllow_Index | FilterAllow_Vertex
	};

	class ScenePrimitivesFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		using QSortFilterProxyModel::QSortFilterProxyModel;

		[[nodiscard]] int getFilterMask() const;
		void setAllowAll();
		void setAllowNone();
		void addFilterEntry(ScenePrimitivesFilterEntry entry);
		void removeFilterEntry(ScenePrimitivesFilterEntry entry);
		bool isVertexFormatAllowed(gamelib::prm::PRMVertexBufferFormat vertexBufferFormat);
		void setVertexFormatAllowed(gamelib::prm::PRMVertexBufferFormat vertexBufferFormat, bool isAllowed);
		void resetToDefaults();

	protected:
		[[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

	private:
		int m_filterMask { 0 };
		std::set<gamelib::prm::PRMVertexBufferFormat> m_allowedVertexFormats { g_DefaultVertexFormats.begin(), g_DefaultVertexFormats.end() };
	};
}