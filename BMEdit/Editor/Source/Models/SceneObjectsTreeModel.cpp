#include <Models/SceneObjectsTreeModel.h>


namespace models
{
	using namespace gamelib::gms;
	using gamelib::scene::SceneObject;

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

		const auto* so = reinterpret_cast<const SceneObject*>(index.constInternalPointer());
		if (!so) return {};

		return QVariant(QString::fromStdString(so->getName()));
	}

	QModelIndex SceneObjectsTreeModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (!hasIndex(row, column, parent) || !isValidLevel())
		{
			return QModelIndex {};
		}

		SceneObject* so = nullptr;
		if (!parent.isValid())
		{
			so = m_level->getSceneObjects()[0].get();
		}
		else
		{
			so = static_cast<SceneObject*>(parent.internalPointer());
		}

		if (row >= 0 && row < so->getChildren().size())
		{
			return createIndex(row, column, (const void*)so->getChildren()[row].lock().get());
		}

		return {};
	}

	QModelIndex SceneObjectsTreeModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid() || !isValidLevel())
		{
			return {};
		}

		auto* child = static_cast<SceneObject*>(index.internalPointer());
		if (auto parent = child->getParent().lock())
		{
			if (parent == m_level->getSceneObjects()[0])
			{
				return {};
			}
			else
			{
				int row = 0;

				for (int i = 0; i < parent->getChildren().size(); ++i)
				{
					if (parent->getChildren()[i].lock().get() == child)
					{
						row = i;
						break;
					}
				}

				return createIndex(row, 0, (const void*)parent.get());
			}
		}

		return {};
	}

	int SceneObjectsTreeModel::rowCount(const QModelIndex &parent) const
	{
		if (!isValidLevel()) return 0;

		if (!parent.isValid())
			return m_level && !m_level->getSceneObjects().empty() ? static_cast<int>(m_level->getSceneObjects()[0]->getChildren().size()) : 0;

		return static_cast<int>(static_cast<SceneObject*>(parent.internalPointer())->getChildren().size());
	}

	int SceneObjectsTreeModel::columnCount(const QModelIndex &parent) const
	{
		if (!isValidLevel()) return 0;

		return 1; // IDK
	}

	QVariant SceneObjectsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			return QVariant {"(SUPER ROOT)" };
		}

		return {};
	}

	void SceneObjectsTreeModel::setLevel(const gamelib::Level *level)
	{
		beginResetModel();
		{
			m_level = level;
		}
		endResetModel();
	}

	bool SceneObjectsTreeModel::isValidLevel() const
	{
		return m_level && m_level->getSceneProperties();
	}
}