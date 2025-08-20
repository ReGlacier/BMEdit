#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#include <QMap>


namespace models
{
	class SceneTextureFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		SceneTextureFilterModel(QObject* parent = nullptr);

		void setTextureNameFilter(const QString& query);
		[[nodiscard]] const QString& getTextureNameFilter() const;

	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

	private:
		QString m_textureNameFilter {};
		mutable QMap<QModelIndex, bool> m_filterResultsCache;
	};
}