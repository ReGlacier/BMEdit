#include <Delegates/ScenePropertyTypeDelegate.h>
#include <Widgets/ScenePropertyWidget.h>
#include <Types/QSceneProperty.h>
#include <QPainter>


using namespace delegates;


ScenePropertyTypeDelegate::ScenePropertyTypeDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *ScenePropertyTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (!index.data(Qt::EditRole).canConvert<types::QSceneProperty>())
	{
		return BaseClass::createEditor(parent, option, index);
	}

	auto editor = new widgets::ScenePropertyWidget(parent);
	editor->setValue(index.data(Qt::EditRole).value<types::QSceneProperty>());

	connect(editor, &widgets::ScenePropertyWidget::valueChanged, this, &ScenePropertyTypeDelegate::commitDataChunk);

	return editor;
}

void ScenePropertyTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	auto ed = qobject_cast<widgets::ScenePropertyWidget*>(editor);

	if (ed && index.data(Qt::EditRole).canConvert<types::QSceneProperty>())
	{
		auto val = qvariant_cast<types::QSceneProperty>(index.data(Qt::EditRole));
		ed->setValue(val);
	}
	else
	{
		QStyledItemDelegate::setEditorData(editor, index);
	}
}

void ScenePropertyTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	auto ed = qobject_cast<widgets::ScenePropertyWidget*>(editor);
	if (ed && index.data(Qt::EditRole).canConvert<types::QSceneProperty>())
	{
		model->setData(index, QVariant::fromValue(ed->getValue()));
	} else {
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}

void ScenePropertyTypeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data(Qt::EditRole).canConvert<types::QSceneProperty>())
	{
		painter->save();
		widgets::ScenePropertyWidget::paintPreview(painter, option, index.data(Qt::EditRole).value<types::QSceneProperty>());
		painter->restore();
	}
	else
	{
		BaseClass::paint(painter, option, index);
	}
}

void ScenePropertyTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (auto ed = qobject_cast<widgets::ScenePropertyWidget*>(editor))
	{
		ed->setGeometry(option.rect);
	}
}

void ScenePropertyTypeDelegate::commitDataChunk()
{
	auto editor = qobject_cast<widgets::ScenePropertyWidget*>(sender());
	emit commitData(editor);
}