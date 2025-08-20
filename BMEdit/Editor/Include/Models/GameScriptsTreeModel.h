#pragma once

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QList>


namespace models
{
	class GameScriptsTreeModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		struct ScripTreeNode
		{
			enum class NodeType {
				STN_ROOT,   /// < Just a ROOT subject. No name, no selector
				STN_BONE,   /// < Bone (temp node). Unable to use as real path
				STN_SCRIPT  /// < Script itself. Could be used as final value
			};

			NodeType type { NodeType::STN_ROOT };
			QString name {};
			QString fullPath {};
			QWeakPointer<struct ScripTreeNode> parent {};
			QList<QSharedPointer<struct ScripTreeNode>> children {};

			ScripTreeNode();
			ScripTreeNode(NodeType nt, QString&& sn ,QString&& sfn);
		};

	public:
		explicit GameScriptsTreeModel(QObject *parent = nullptr);

		QVariant data(const QModelIndex &index, int role) const override;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex &index) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	private:
		void buildTree();

	private:
		QSharedPointer<ScripTreeNode> m_root { nullptr };
	};
}