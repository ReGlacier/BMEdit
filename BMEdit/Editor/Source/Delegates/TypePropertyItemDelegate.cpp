#include <Delegates/TypePropertyItemDelegate.h>
#include <Widgets/TypeVector3PropertyWidget.h>
#include <Widgets/TypeMatrixPropertyWidget.h>
#include <Widgets/TypeSimplePropertyWidget.h>
#include <Widgets/TypeRefTabPropertyWidget.h>
#include <Widgets/TypePropertyWidget.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <Types/QGlacierValue.h>
#include <GameLib/Type.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QPainter>


namespace delegates
{
	enum Matrix3x3 : int { Rows = 3, Columns = 3 };

	static bool isValueCouldBePresentedBySimpleView(const types::QGlacierValue &data)
	{
		if (data.instructions.size() == 1)
		{
			const auto& instruction = data.instructions[0];
			return instruction.isEnum() || instruction.isTrivialValue();
		}

		return false;
	}

	static bool isValueCouldBePresentedAsSimpleVectorWidget(const types::QGlacierValue &data)
	{
		if (data.instructions.size() == 5)
		{
			return data.instructions.front().isBeginArray() && data.instructions.back().isEndArray() && data.instructions.front().getOperand().trivial.i32 == 3;
		}

		return false;
	}

	static bool isValueCouldBePresentedAsSimpleMatrixWidget(const types::QGlacierValue &data, int rows, int columns)
	{
		if (data.instructions.size() == (2 + (rows * columns)))
		{
			return data.instructions.front().isBeginArray() && data.instructions.back().isEndArray() && data.instructions.front().getOperand().trivial.i32 == (rows * columns);
		}

		return false;
	}

	static bool isRefTab(const types::QGlacierValue &data)
	{
		if (data.views.empty())
			return false;

		const gamelib::ValueView& view = data.views.at(0);
		if (auto type = view.getType())
		{
			return (type->getKind() == gamelib::TypeKind::CONTAINER) && (type->getName().find("ZREFTAB") != std::string::npos);
		}

		return false;
	}

	TypePropertyItemDelegate::TypePropertyItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
	{
	}

	QWidget *TypePropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			widgets::TypePropertyWidget *editor = nullptr;

			auto data = index.data(Qt::EditRole).value<types::QGlacierValue>();
			if (data.instructions.empty())
			{
				// Empty widget
				editor = new widgets::TypePropertyWidget(parent);
			}
			else if (isValueCouldBePresentedBySimpleView(data))
			{
				// If trivial thing (int, string, enum (?), bool) we may show simple widgets::TypeSimplePropertyWidget (inherited of widgets::TypePropertyWidget)
				editor = new widgets::TypeSimplePropertyWidget(parent);
			}
			else if (isValueCouldBePresentedAsSimpleVectorWidget(data))
			{
				// It's vector (3 elements)
				editor = new widgets::TypeVector3PropertyWidget(parent);
			}
			else if (isValueCouldBePresentedAsSimpleMatrixWidget(data, Matrix3x3::Rows, Matrix3x3::Columns))
			{
				// It's matrix 3x3
				editor = new widgets::TypeMatrixPropertyWidget(Matrix3x3::Rows, Matrix3x3::Columns, parent);
			}
			else if (isRefTab(data))
			{
				editor = new widgets::TypeRefTabPropertyWidget(parent);
			}

			if (editor)
			{
				editor->setValue(data);
				connect(editor, &widgets::TypePropertyWidget::valueChanged, this, &TypePropertyItemDelegate::commitDataChunk);
				connect(editor, &widgets::TypePropertyWidget::editFinished, this, &TypePropertyItemDelegate::commitDataChunkAndCloseEditor);
				return editor;
			}
			else
			{
				auto res = new QLabel(QString("UNSUPPORTED"), parent);
				res->setAutoFillBackground(true);
				res->setStyleSheet("QLabel { background-color : red; color : blue; }");
				return res;
			}

			return nullptr;
		}

		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void TypePropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor);

		if (ed && index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			auto val = qvariant_cast<types::QGlacierValue>(index.data(Qt::EditRole));
			ed->setValue(val);
		}
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

	void TypePropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor);
		if (ed && index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			model->setData(index, QVariant::fromValue(ed->getValue()));
		} else {
			QStyledItemDelegate::setModelData(editor, model, index);
		}
	}

	void TypePropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (index.data(Qt::EditRole).canConvert<types::QGlacierValue>())
		{
			auto data = index.data(Qt::EditRole).value<types::QGlacierValue>();
			if (isValueCouldBePresentedBySimpleView(data))
			{
				painter->save();
				widgets::TypeSimplePropertyWidget::paintPreview(painter, option, data);
				painter->restore();
				return;
			}
			else if (isValueCouldBePresentedAsSimpleVectorWidget(data))
			{
				painter->save();
				widgets::TypeVector3PropertyWidget::paintPreview(painter, option, data);
				painter->restore();
				return;
			}
			else if (isValueCouldBePresentedAsSimpleMatrixWidget(data, Matrix3x3::Rows, Matrix3x3::Columns))
			{
				painter->save();
				widgets::TypeMatrixPropertyWidget::paintPreview(Matrix3x3::Rows, Matrix3x3::Columns, painter, option, data);
				painter->restore();
				return;
			}
			else if (isRefTab(data))
			{
				painter->save();
				widgets::TypeRefTabPropertyWidget::paintPreview(painter, option, data);
				painter->restore();
				return;
			}
		}

		BaseClass::paint(painter, option, index);
	}

	void TypePropertyItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (auto ed = qobject_cast<widgets::TypePropertyWidget*>(editor))
		{
			ed->setGeometry(option.rect);
		}
	}

	void TypePropertyItemDelegate::commitDataChunk()
	{
		auto editor = qobject_cast<widgets::TypePropertyWidget*>(sender());
		emit commitData(editor);
	}

	void TypePropertyItemDelegate::commitDataChunkAndCloseEditor()
	{
		auto editor = qobject_cast<widgets::TypePropertyWidget*>(sender());
		emit commitData(editor);
		emit closeEditor(editor, QAbstractItemDelegate::EndEditHint::SubmitModelCache);
	}
}