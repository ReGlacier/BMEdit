#pragma once

#include <QAbstractListModel>


namespace gamelib::scene
{
	class SceneObject;
}

namespace models
{
	class GeomControllerListModel : public QAbstractListModel
	{
		Q_OBJECT

	public:
		GeomControllerListModel(QObject *parent = nullptr);

		Qt::ItemFlags flags(const QModelIndex &index) const override;
	    QModelIndex	index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;

		void setGeom(gamelib::scene::SceneObject *sceneObject);
		void resetGeom();

	private:
		[[nodiscard]] bool isReady() const;

	private:
		gamelib::scene::SceneObject *m_sceneObject { nullptr };
	};
}