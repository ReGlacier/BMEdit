#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#include <QMap>


namespace models
{
	class SceneFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		SceneFilterModel(QObject* parent = nullptr);

		void setQuery(const QString& query);
		[[nodiscard]] const QString& getQuery() const;

	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

	private:
		QString m_query {};
		mutable QMap<QModelIndex, bool> m_filterResultsCache;
	};
}