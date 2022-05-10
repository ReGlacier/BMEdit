#pragma once

#include <QAbstractItemModel>

#include <GameLib/Level.h>


namespace models
{
	class SceneObjectsTreeModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		explicit SceneObjectsTreeModel(QObject *parent = nullptr);
		explicit SceneObjectsTreeModel(const gamelib::Level *level, QObject *parent = nullptr);

		QVariant data(const QModelIndex &index, int role) const override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex &index) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

		void setLevel(const gamelib::Level *level);

	private:
		[[nodiscard]] bool isValidLevel() const;
		void loadLevelIndices();

	public:
		struct Index
		{
			const gamelib::gms::GMSGeomEntity *data { nullptr };
			struct Index *parent { nullptr };
			std::vector<struct Index *> children {};
			uint32_t depth { 0u };
			std::size_t sceneIndex { 0 };

			[[nodiscard]] int childCount() const { return children.size(); }
			[[nodiscard]] int row() const {
				if (parent)
				{
					auto it = std::find(parent->children.begin(), parent->children.end(), this);
					return it - parent->children.begin();
				}

				return 0;
			}
			[[nodiscard]] int columnCount() const {
				return 1;
			}
			[[nodiscard]] QVariant getQData() const {
				if (!data) {
					return QVariant();
				}

				return QVariant(QString::fromStdString(data->getName()));
			}
		};

	private:
		const gamelib::Level *m_level { nullptr };
		std::vector<Index> m_indices;
	};
}