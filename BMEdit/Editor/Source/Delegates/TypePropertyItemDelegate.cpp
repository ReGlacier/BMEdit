#include <Delegates/TypePropertyItemDelegate.h>
#include <Widgets/TypePropertyWidget.h>
#include <Types/QGlacierValue.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QPainter>


namespace delegates
{
	TypePropertyItemDelegate::TypePropertyItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
	{
	}

	QWidget *TypePropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			auto editor = new widgets::TypePropertyWidget(parent);
			connect(editor, &widgets::TypePropertyWidget::valueChanged, this, &TypePropertyItemDelegate::commitDataChunk);

			return editor;
		}

		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void TypePropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		if (index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			auto val = qvariant_cast<types::QGlacierValue>(index.data(Qt::EditRole));
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
		if (index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor);
			if (ed)
			{
				model->setData(index, QVariant::fromValue(ed->getValue()));
			}
		} else {
			QStyledItemDelegate::setModelData(editor, model, index);
		}
	}

	void TypePropertyItemDelegate::commitDataChunk()
	{
		auto editor = qobject_cast<widgets::TypePropertyWidget*>(sender());
		emit commitData(editor);
	}
}