#pragma once

#include <QWidget>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <Types/QSceneProperty.h>


namespace widgets
{
	class ScenePropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		ScenePropertyWidget(QWidget *parent = nullptr);

		void setValue(const types::QSceneProperty &value);
		[[nodiscard]] const types::QSceneProperty &getValue() const;

		static void paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QSceneProperty &value);

	signals:
		void valueChanged();
		void editFinished();

	protected:
		static bool areSame(const types::QSceneProperty &current, const types::QSceneProperty &value);

		void buildLayout(const types::QSceneProperty &value);
		void updateLayout(const types::QSceneProperty &value);

	protected:
		types::QSceneProperty m_value {};
	};
}