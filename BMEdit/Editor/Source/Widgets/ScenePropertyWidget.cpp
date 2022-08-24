#include <Widgets/ScenePropertyWidget.h>
#include <QStringList>

using namespace widgets;


ScenePropertyWidget::ScenePropertyWidget(QWidget *parent) : QWidget(parent)
{
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

void ScenePropertyWidget::paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QSceneProperty &value)
{
	// In theory, we need to make switch/case by types and more asserts, but it's preview, who cares?
	StaticRenderVisitorValue renderer { painter, option };
	std::visit(renderer, value.def.getValue());
}

void ScenePropertyWidget::buildLayout(const types::QSceneProperty &value)
{
	//TODO: Implement me
}

void ScenePropertyWidget::updateLayout(const types::QSceneProperty &value)
{
	//TODO: Implement me
}