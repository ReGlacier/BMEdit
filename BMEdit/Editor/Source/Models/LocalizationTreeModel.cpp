#include <Models/LocalizationTreeModel.h>


namespace models
{
	LocalizationTreeModel::LocalizationTreeModel(QObject *parent) : QAbstractItemModel(parent)
	{
	}

	LocalizationTreeModel::LocalizationTreeModel(const gamelib::Level *level, QObject *parent) : QAbstractItemModel(parent)
	{
		setLevel(level);
	}

	QVariant LocalizationTreeModel::data(const QModelIndex &index, int role) const
	{
		if (!isValidLevel())
		{
			return QVariant {};
		}

		const auto* node = reinterpret_cast<const gamelib::loc::LOCTreeNode*>(index.constInternalPointer());
		if (!node) return {};

		if (role == Qt::DisplayRole)
		{
			if (index.column() == 0) return QString::fromStdString(node->name);
			if (index.column() == 1 && (node->type == gamelib::loc::LOCTreeNodeType::LOCALIZED_STRING || node->type == gamelib::loc::LOCTreeNodeType::SUBTITLES))
			{
				return QString::fromStdString(node->value);
			}

			return {};
		}

		if (role == Qt::ItemDataRole::ToolTipRole)
		{
			QStringList locationPath {};

			auto currentNode = node;
			while (currentNode)
			{
				locationPath.push_front(QString::fromStdString(currentNode->name));
				const auto& parent = currentNode->parent.lock();
				currentNode = parent ? parent.get() : nullptr;
			}

			return locationPath.join('/');
		}

		return {};
	}

	QModelIndex LocalizationTreeModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (!hasIndex(row, column, parent) || !isValidLevel())
		{
			return QModelIndex {};
		}

		gamelib::loc::LOCTreeNode* node = nullptr;
		if (!parent.isValid())
		{
			node = m_level->getLevelLocalization()->localizationRoot.get();
		}
		else
		{
			node = static_cast<gamelib::loc::LOCTreeNode*>(parent.internalPointer());
		}

		if (row >= 0 && row < node->children.size())
		{
			return createIndex(row, column, (const void*)node->children[row].get());
		}

		return {};
	}

	QModelIndex LocalizationTreeModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid() || !isValidLevel())
		{
			return {};
		}

		auto* child = static_cast<gamelib::loc::LOCTreeNode*>(index.internalPointer());
		if (auto parent = child->parent.lock())
		{
			if (parent == m_level->getLevelLocalization()->localizationRoot)
			{
				return {};
			}
			else
			{
				int row = 0;

				for (int i = 0; i < parent->children.size(); ++i)
				{
					if (parent->children[i].get() == child)
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

	int LocalizationTreeModel::rowCount(const QModelIndex &parent) const
	{
		if (!isValidLevel()) return 0;

		if (!parent.isValid())
		{
			const auto& root = m_level->getLevelLocalization()->localizationRoot;

			return root->children.empty() ? 0 : static_cast<int>(root->children.size());
		}

		if (auto node = static_cast<gamelib::loc::LOCTreeNode*>(parent.internalPointer()))
		{
			if (node->type == gamelib::loc::CHILDREN)
				return static_cast<int>(node->children.size());

			return 0; // only value
		}

		return 0;
	}

	int LocalizationTreeModel::columnCount(const QModelIndex &parent) const
	{
		if (!isValidLevel()) return 0;
		return 2; // 1 for tree, 1 for value
	}

	QVariant LocalizationTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
		{
			if (section == 0) return "Tree";
			if (section == 1) return "Value";
		}

		return QAbstractItemModel::headerData(section, orientation, role);
	}

	void LocalizationTreeModel::setLevel(const gamelib::Level *level)
	{
		beginResetModel();
		m_level = level;
		endResetModel();
	}

	void LocalizationTreeModel::resetLevel()
	{
		beginResetModel();
		m_level = nullptr;
		endResetModel();
	}

	QModelIndex LocalizationTreeModel::getRootIndex() const
	{
		if (!m_level || !m_level->getLevelLocalization())
		{
			return {};
		}

		return createIndex(0, 0, (const void*)m_level->getLevelLocalization()->localizationRoot.get());
	}

	bool LocalizationTreeModel::isValidLevel() const
	{
		return m_level && m_level->getLevelLocalization();
	}
}