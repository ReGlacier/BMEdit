#include <Models/SceneObjectsTreeModel.h>


namespace models
{
	using namespace gamelib::gms;

	SceneObjectsTreeModel::SceneObjectsTreeModel(QObject *parent)
		: QAbstractItemModel(parent)
	{
	}

	SceneObjectsTreeModel::SceneObjectsTreeModel(const gamelib::Level *level, QObject *parent)
		: QAbstractItemModel(parent)
	{
		setLevel(level);
	}

	QVariant SceneObjectsTreeModel::data(const QModelIndex &index, int role) const
	{
		if (role != Qt::DisplayRole || !isValidLevel())
		{
			return QVariant {};
		}

		const auto *entityIndex = reinterpret_cast<const Index *>(index.constInternalPointer());
		if (!entityIndex || !entityIndex->data)
		{
			return QVariant {};
		}

		return QVariant(QString::fromStdString(entityIndex->data->getName()));
	}

	QModelIndex SceneObjectsTreeModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (!hasIndex(row, column, parent) || !isValidLevel())
		{
			return QModelIndex {};
		}

		const Index *parentIndex = nullptr;
		if (!parent.isValid())
		{
			parentIndex = &m_indices[0];
		} else {
			parentIndex = reinterpret_cast<const Index *>(parent.constInternalPointer());
		}

		const Index *childIndex = nullptr;

		if (row >= 0 && row < parentIndex->children.size()) {
			childIndex = parentIndex->children[row];
			return createIndex(row, column, childIndex);
		}

		return QModelIndex {};
	}

	QModelIndex SceneObjectsTreeModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid() || !isValidLevel())
		{
			return QModelIndex {};
		}

		const Index *childIndex = reinterpret_cast<const Index *>(index.constInternalPointer());
		const Index *parentIndex = childIndex->parent;

		if (!parentIndex || parentIndex == &m_indices[0])
		{
			return QModelIndex {};
		}

		return createIndex(parentIndex->row(), 0, parentIndex);
	}

	int SceneObjectsTreeModel::rowCount(const QModelIndex &parent) const
	{
		if (!isValidLevel())
		{
			return 0;
		}

		const Index *parentIndex = nullptr;

		if (parent.column() > 0)
		{
			return 0;
		}

		if (!parent.isValid())
		{
			parentIndex = &m_indices[0];
		}
		else
		{
			parentIndex = reinterpret_cast<const Index *>(parent.constInternalPointer());
		}

		return parentIndex->childCount();
	}

	int SceneObjectsTreeModel::columnCount(const QModelIndex &parent) const
	{
		if (!isValidLevel())
		{
			return 0;
		}

		if (parent.isValid())
		{
			return reinterpret_cast<const Index *>(parent.constInternalPointer())->columnCount();
		}

		return m_indices[0].columnCount();
	}

	QVariant SceneObjectsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			return QVariant {"Game Object" };
		}

		return {};
	}

	void SceneObjectsTreeModel::setLevel(const gamelib::Level *level)
	{
		beginResetModel();
		{
			m_level = level;
			m_indices.clear();

			loadLevelIndices();
		}
		endResetModel();
	}

	bool SceneObjectsTreeModel::isValidLevel() const
	{
		return m_level && m_level->getSceneProperties();
	}

	void SceneObjectsTreeModel::loadLevelIndices()
	{
		if (isValidLevel())
		{
			const auto &entities = m_level->getSceneProperties()->header.getEntries().getGeomEntities();
			if (!entities.empty())
			{
				m_indices.resize(entities.size());

				// Iterate over entities
				for (int entityIndex = 0; entityIndex < entities.size(); ++entityIndex)
				{
					auto &currentIndex = m_indices[entityIndex];
					const auto &currentEntity = entities[entityIndex];

					currentIndex.data = &currentEntity;
					currentIndex.sceneIndex = entityIndex;

					if (currentEntity.getParentGeomIndex() != GMSGeomEntity::kInvalidParent)
					{
						currentIndex.parent = &m_indices[currentEntity.getParentGeomIndex()];
						m_indices[currentEntity.getParentGeomIndex()].children.push_back(&currentIndex);
					}
				}
			}
		}
	}
}