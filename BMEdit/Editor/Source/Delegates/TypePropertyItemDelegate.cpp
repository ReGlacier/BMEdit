#include <Delegates/TypePropertyItemDelegate.h>
#include <Widgets/TypePropertyWidget.h>
#include <Types/QGlacierValue.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>


namespace delegates
{
	TypePropertyItemDelegate::TypePropertyItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
	{
	}

	void TypePropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.data().canConvert<types::QGlacierValue>())
		{
			//TODO: Impl for us!
		}
		else
		{
			QStyledItemDelegate::paint(painter, option, index);
		}
	}

	QSize TypePropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.data().canConvert<types::QGlacierValue>())
		{
			//auto v = qvariant_cast<types::QGlacierValue>(index.data());
			//TODO: Here we need to check internal type, count of properties and determine which size of widget required here
			//      For now I will return 100x10
			return QSize(100, 10);
		}

		return QStyledItemDelegate::sizeHint(option, index);
	}

	QWidget *TypePropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		return new QPushButton("PUSH", parent);
	}

	void TypePropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		if (index.data().canConvert<types::QGlacierValue>())
		{
			auto val = qvariant_cast<types::QGlacierValue>(index.data());
			auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor);

			ed->setValue(val);
		}
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

	void TypePropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		if (index.data().canConvert<types::QGlacierValue>())
		{
			auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor);
			model->setData(index, QVariant::fromValue(ed->getValue()));
		} else {
			QStyledItemDelegate::setModelData(editor, model, index);
		}
	}
}