#pragma once

#include <QAbstractItemModel>

#include <GameLib/Level.h>


namespace models
{
	class LocalizationTreeModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		explicit LocalizationTreeModel(QObject *parent = nullptr);
		explicit LocalizationTreeModel(const gamelib::Level *level, QObject *parent = nullptr);

		QVariant data(const QModelIndex &index, int role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex &index) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void setLevel(const gamelib::Level *level);
		void resetLevel();
		QModelIndex getRootIndex() const;

	private:
		[[nodiscard]] bool isValidLevel() const;

	private:
		const gamelib::Level *m_level { nullptr };
	};
}