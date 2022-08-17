#pragma once

#include <QWidget>
#include <Types/QGlacierValue.h>


namespace widgets
{
	/*
	 * This widget representing inside data as one (or more) standard Qt widgets (QEdit, QLabel, Q...Matrix? Bruh)
	 * TODO: Impl this please)))0)
	 */
	class TypePropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		TypePropertyWidget(QWidget* parent = nullptr);

		void setValue(const types::QGlacierValue &value);
		[[nodiscard]] const types::QGlacierValue &getValue() const;

		QSize sizeHint() const override;

	private:
		void rebuildLayout();

	private:
		types::QGlacierValue m_value {};
		QSize m_minSize { 10, 10 };
	};
}