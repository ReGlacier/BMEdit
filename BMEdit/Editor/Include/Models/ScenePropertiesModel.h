#pragma once

#include <QAbstractItemModel>
#include <GameLib/PRP/PRPZDefines.h>
#include <optional>


namespace gamelib
{
	class Level;
}

namespace models
{
	class ScenePropertiesModel: public QAbstractTableModel
	{
		Q_OBJECT

	protected:
		enum ColumnID : int {
			NAME = 0,
			VALUE = 1,

			MAX_COLUMNS
		};

	public:
		ScenePropertiesModel(QObject *parent = nullptr);

		int rowCount(const QModelIndex &parent) const override;
		int columnCount(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void setLevel(gamelib::Level *level);
		void resetLevel();

	signals:
		void valueChanged();

	private:
		bool isReady() const;

	private:
		gamelib::Level *m_level { nullptr };
	};
}