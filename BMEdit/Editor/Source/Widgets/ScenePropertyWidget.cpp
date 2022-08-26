#include <Widgets/ScenePropertyWidget.h>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>

using namespace widgets;

namespace visitors
{
	struct StaticRenderVisitorValue
	{
		QPainter *painter;
		const QStyleOptionViewItem &options;

		void operator()(const gamelib::prp::ArrayI32 &i32)
		{
			if (i32.empty()) return;

			QTextOption textOption;
			textOption.setAlignment(Qt::AlignCenter);

			QStringList valuesList {};
			valuesList.reserve(static_cast<qsizetype>(i32.size()));

			for (const auto &v : i32)
			{
				valuesList.push_back(QString("%1").arg(v));
			}

			painter->drawText(options.rect, valuesList.join(','), textOption);
		}

		void operator()(const gamelib::prp::ArrayF32 &f32)
		{
			if (f32.empty()) return;

			QTextOption textOption;
			textOption.setAlignment(Qt::AlignCenter);

			QStringList valuesList {};
			valuesList.reserve(static_cast<qsizetype>(f32.size()));

			for (const auto &v : f32)
			{
				valuesList.push_back(QString("%1").arg(v));
			}

			painter->drawText(options.rect, valuesList.join(','), textOption);
		}

		void operator()(const gamelib::prp::StringRef &strRef)
		{
			QTextOption textOption;
			textOption.setAlignment(Qt::AlignCenter);

			if (strRef.empty())
			{
				painter->drawText(options.rect, QString("(empty)"), textOption);
			}
			else
			{
				painter->drawText(options.rect, QString::fromStdString(strRef), textOption);
			}
		}

		void operator()(const gamelib::prp::StringRefTab &stringRefTab)
		{
			if (stringRefTab.empty()) return;

			QTextOption textOption;
			textOption.setAlignment(Qt::AlignCenter);

			QStringList valuesList {};
			valuesList.reserve(static_cast<qsizetype>(stringRefTab.size()));

			for (const auto &v : stringRefTab)
			{
				valuesList.push_back(QString::fromStdString(v));
			}

			painter->drawText(options.rect, valuesList.join('\n'), textOption);
		}
	};

	class WidgetCreatorVisitor
	{
		ScenePropertyWidget *m_parent { nullptr };

	public:
		explicit WidgetCreatorVisitor(ScenePropertyWidget *parent) : m_parent(parent)
		{
		}

		void operator()(const gamelib::prp::ArrayI32 &i32)
		{
		}

		void operator()(const gamelib::prp::ArrayF32 &f32)
		{
		}

		void operator()(const gamelib::prp::StringRef &strRef)
		{
			auto lay = new QVBoxLayout(m_parent);
			auto edit = new QLineEdit(m_parent);
			edit->setAccessibleName("StringRefEditor");
			edit->setText(QString::fromStdString(strRef));

			QWidget::connect(edit, &QLineEdit::textChanged, [parent = m_parent](const QString &newValue) { // Capture only parent!!!
				if (gamelib::prp::StringRef *vPtr = std::get_if<gamelib::prp::StringRef>(&parent->m_value.def.getValue()))
				{
					*vPtr = newValue.toStdString();
					emit parent->valueChanged();
				}
			});

			lay->setContentsMargins(0, 0, 0, 0);
			lay->addWidget(edit);
			m_parent->setLayout(lay);
		}

		void operator()(const gamelib::prp::StringRefTab &stringRefTab)
		{
		}
	};

	class WidgetUpdaterVisitor
	{
		ScenePropertyWidget *m_parent { nullptr };

	public:
		explicit WidgetUpdaterVisitor(ScenePropertyWidget *parent) : m_parent(parent)
		{
		}

		void operator()(const gamelib::prp::ArrayI32 &i32)
		{
		}

		void operator()(const gamelib::prp::ArrayF32 &f32)
		{
		}

		void operator()(const gamelib::prp::StringRef &strRef)
		{
			if (auto edit = m_parent->findChild<QLineEdit*>("StringRefEditor"))
			{
				QSignalBlocker blocker { edit };
				edit->setText(QString::fromStdString(strRef));
			}
		}

		void operator()(const gamelib::prp::StringRefTab &stringRefTab)
		{
		}
	};
}


ScenePropertyWidget::ScenePropertyWidget(QWidget *parent) : QWidget(parent)
{
	setAutoFillBackground(true);
}

void ScenePropertyWidget::setValue(const types::QSceneProperty &value)
{
	if (!areSame(m_value, value))
	{
		buildLayout(value);
	}
	else
	{
		updateLayout(value);
	}

	m_value = value;
}

bool ScenePropertyWidget::areSame(const types::QSceneProperty &current, const types::QSceneProperty &value)
{
	return current.def == value.def;
}

const types::QSceneProperty &ScenePropertyWidget::getValue() const
{
	return m_value;
}

void ScenePropertyWidget::paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QSceneProperty &value)
{
	// In theory, we need to make switch/case by types and more asserts, but it's preview, who cares?
	visitors::StaticRenderVisitorValue renderer { painter, option };
	std::visit(renderer, value.def.getValue());
}

void ScenePropertyWidget::buildLayout(const types::QSceneProperty &value)
{
	visitors::WidgetCreatorVisitor producer(this);
	std::visit(producer, value.def.getValue());
}

void ScenePropertyWidget::updateLayout(const types::QSceneProperty &value)
{
	visitors::WidgetUpdaterVisitor updater(this);
	std::visit(updater, value.def.getValue());
}