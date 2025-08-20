#include <Models/GameScriptsTreeModel.h>
#include <GameLib/TypeRegistry.h>
#include <QIcon>


namespace models
{
	GameScriptsTreeModel::ScripTreeNode::ScripTreeNode() = default;

	GameScriptsTreeModel::ScripTreeNode::ScripTreeNode(models::GameScriptsTreeModel::ScripTreeNode::NodeType nt, QString &&sn, QString &&sfn)
		: type(nt), name(std::move(sn)), fullPath(std::move(sfn))
	{
	}

	GameScriptsTreeModel::GameScriptsTreeModel(QObject *parent) : QAbstractItemModel(parent)
	{
		buildTree();
	}

	QVariant GameScriptsTreeModel::data(const QModelIndex &index, int role) const
	{
		if (!m_root) return {};

		const auto* pScript = reinterpret_cast<const ScripTreeNode*>(index.constInternalPointer());
		if (!pScript) return {};

		if (role == Qt::DisplayRole)
		{
			return pScript->name;
		}

		if (role == Qt::DecorationRole)
		{
			static QIcon kFolderIcon(":/bmedit/folder_icon.png");
			static QIcon kScriptIcon(":/bmedit/script_icon.png");
			static QIcon kUnknownIcon(":/bmedit/unknown_icon.png");

			switch (pScript->type)
			{
				case ScripTreeNode::NodeType::STN_ROOT:
				case ScripTreeNode::NodeType::STN_BONE:
				    return kFolderIcon;
				    break;
				case ScripTreeNode::NodeType::STN_SCRIPT:
				    return kScriptIcon;
			}

			return kUnknownIcon;
		}

		if (role == Qt::ItemDataRole::ToolTipRole)
		{
			return pScript->fullPath;
		}

		return {};
	}

	QModelIndex GameScriptsTreeModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (!hasIndex(row, column, parent) || !m_root)
		{
			return QModelIndex {};
		}

		ScripTreeNode* pScript = nullptr;
		if (!parent.isValid())
		{
			pScript = m_root.get();
		}
		else
		{
			pScript = static_cast<ScripTreeNode*>(parent.internalPointer());
		}

		if (row >= 0 && row < pScript->children.size())
		{
			return createIndex(row, column, (const void*)pScript->children[row].get());
		}

		return {};
	}

	QModelIndex GameScriptsTreeModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid() || !m_root)
		{
			return {};
		}

		auto* child = static_cast<ScripTreeNode*>(index.internalPointer());
		if (auto parent = child->parent.lock())
		{
			if (parent == m_root)
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

	int GameScriptsTreeModel::rowCount(const QModelIndex &parent) const
	{
		if (!m_root)
		{
			return 0;
		}

		if (!parent.isValid())
		{
			return m_root && !m_root->children.empty() ? static_cast<int>(m_root->children.size()) : 0;
		}

		return static_cast<int>(static_cast<ScripTreeNode*>(parent.internalPointer())->children.size());
	}

	int GameScriptsTreeModel::columnCount(const QModelIndex &parent) const
	{
		return m_root ? 1 : 0;
	}

	QVariant GameScriptsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		return m_root ? QVariant::fromValue(QString("Game Scripts")) : QVariant {};
	}

	void insertIntoTree(const QString& str, const QSharedPointer<GameScriptsTreeModel::ScripTreeNode>& root)
	{
		// Split the string into parts based on the separator
		QStringList parts = str.split('\\');

		// Start from the root
		QSharedPointer<GameScriptsTreeModel::ScripTreeNode> current = root;

		for (int i = 0; i < parts.size(); ++i) {
			bool found = false;
			// Check if the current part already exists as a child
			foreach (const QSharedPointer<GameScriptsTreeModel::ScripTreeNode>& child, current->children) {
				if (child->name == parts[i]) {
					current = child;
					found = true;
					break;
				}
			}

			// If the part is not found, create a new node
			if (!found) {
				GameScriptsTreeModel::ScripTreeNode::NodeType type = (i < parts.size() - 1) ? GameScriptsTreeModel::ScripTreeNode::NodeType::STN_BONE : GameScriptsTreeModel::ScripTreeNode::NodeType::STN_SCRIPT;
				QString fullPath = (current != root) ? current->fullPath + '\\' + parts[i] : parts[i];
				QSharedPointer<GameScriptsTreeModel::ScripTreeNode> newNode(new GameScriptsTreeModel::ScripTreeNode(type, std::move(parts[i]), std::move(fullPath)));
				newNode->parent = current;
				current->children.append(newNode);
				current = newNode;
			}
		}
	}

	void GameScriptsTreeModel::buildTree()
	{
		m_root = QSharedPointer<ScripTreeNode>::create(ScripTreeNode::NodeType::STN_ROOT, "ROOT", "ROOT");

		gamelib::TypeRegistry::getInstance().forEachScript([this](const std::string& name, const gamelib::ScriptInfo& /*info*/) {
			insertIntoTree(QString::fromStdString(name), m_root);
		});
	}
}