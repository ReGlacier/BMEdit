#pragma once

#include <GameLib/Level.h>
#include <GameLib/GMS/GMSGeomEntity.h>

#include <QAbstractTableModel>
#include <optional>


namespace models
{
	class SceneObjectPropertiesModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		explicit SceneObjectPropertiesModel(QObject *parent = nullptr);

		int rowCount(const QModelIndex &parent) const override;
		int columnCount(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void setLevel(const gamelib::Level *level);
		void setGeomIndex(std::size_t geomIndex);
		void resetLevel();
		void resetGeomIndex();

	private:
		std::optional<std::size_t> m_geomIndex {};
		const gamelib::Level *m_level { nullptr };
	};
}