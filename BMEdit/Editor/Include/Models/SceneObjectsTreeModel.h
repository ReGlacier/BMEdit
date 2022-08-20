#pragma once

#include <QAbstractItemModel>

#include <GameLib/Level.h>


namespace models
{
	class SceneObjectsTreeModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		static constexpr int SceneObjectRole = Qt::UserRole + 1;

		explicit SceneObjectsTreeModel(QObject *parent = nullptr);
		explicit SceneObjectsTreeModel(const gamelib::Level *level, QObject *parent = nullptr);

		QVariant data(const QModelIndex &index, int role) const override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex &index) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

		void setLevel(const gamelib::Level *level);
		QModelIndex getRootIndex() const;

	private:
		[[nodiscard]] bool isValidLevel() const;

	private:
		const gamelib::Level *m_level { nullptr };
	};
}