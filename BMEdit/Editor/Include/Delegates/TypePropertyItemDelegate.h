#pragma once

#include <QStyledItemDelegate>



namespace delegates
{
	class TypePropertyItemDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

		using BaseClass = QStyledItemDelegate;
	public:
		TypePropertyItemDelegate(QObject *parent = nullptr);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

		void paint(QPainter *painter,
		           const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		void updateEditorGeometry(QWidget *editor,
		                          const QStyleOptionViewItem &option,
		                          const QModelIndex &index) const override;

	private slots:
		void commitDataChunk();
	};
}