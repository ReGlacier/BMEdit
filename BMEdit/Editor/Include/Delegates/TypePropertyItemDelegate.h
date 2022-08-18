#pragma once

#include <QStyledItemDelegate>



namespace delegates
{
	class TypePropertyItemDelegate : public QStyledItemDelegate
	{
		Q_OBJECT
	public:
		TypePropertyItemDelegate(QObject *parent = nullptr);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

	private slots:
		void commitDataChunk();
	};
}