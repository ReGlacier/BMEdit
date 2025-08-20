#include <Models/SceneTextureFilterModel.h>
#include <Models/SceneTexturesModel.h>


namespace models
{
	SceneTextureFilterModel::SceneTextureFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
	{
	}

	void SceneTextureFilterModel::setTextureNameFilter(const QString& query)
	{
		beginResetModel();
		{
			m_textureNameFilter = query;
			m_filterResultsCache.clear();
		}
		endResetModel();
	}

	const QString& SceneTextureFilterModel::getTextureNameFilter() const
	{
		return m_textureNameFilter;
	}

	bool SceneTextureFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const // NOLINT(misc-no-recursion)
	{
		if (m_textureNameFilter.isEmpty())
			return true; // Allow all

		QAbstractItemModel *srcModel = sourceModel();

		if (sourceParent.isValid() && sourceRow >= srcModel->rowCount(sourceParent))
		{
			return false;
		}

		QModelIndex index = srcModel->index(sourceRow, SceneTexturesModel::ColumnID::NAME, sourceParent);
		if (m_filterResultsCache.contains(index))
		{
			return m_filterResultsCache[index];
		}

		// Try build cache
		QRegularExpression regex(m_textureNameFilter, QRegularExpression::CaseInsensitiveOption);
		QString text = srcModel->data(index, Qt::DisplayRole).toString();
		if (regex.match(text).hasMatch())
		{
			m_filterResultsCache[index] = true;
			return true;
		}

		// Or fill false
		m_filterResultsCache[index] = false;
		return false;
	}
}