#include <Models/SceneFilterModel.h>

namespace models
{
	SceneFilterModel::SceneFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
	{
	}

	void SceneFilterModel::setQuery(const QString& query)
	{
		beginResetModel();
		{
			m_query = query;
			m_filterResultsCache.clear();
		}
		endResetModel();
	}

	const QString &SceneFilterModel::getQuery() const
	{
		return m_query;
	}

	bool SceneFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		if (m_query.isEmpty())
			return true; // Allow all

		QAbstractItemModel *srcModel = sourceModel();

		if (sourceParent.isValid() && sourceRow >= srcModel->rowCount(sourceParent))
		{
			return false;
		}

		QModelIndex index = srcModel->index(sourceRow, filterKeyColumn(), sourceParent);

		if (m_filterResultsCache.contains(index))
		{
			return m_filterResultsCache[index];
		}

		QRegularExpression regex(m_query, QRegularExpression::CaseInsensitiveOption);
		QString text = srcModel->data(index, Qt::DisplayRole).toString();
		if (regex.match(text).hasMatch())
		{
			m_filterResultsCache[index] = true;
			return true;
		}

		int childrenNr = srcModel->rowCount(index);
		for (int i = 0; i < childrenNr; ++i)
		{
			if (filterAcceptsRow(i, index))
			{
				m_filterResultsCache[index] = true;
				return true;
			}
		}

		m_filterResultsCache[index] = false;
		return false;
	}
}