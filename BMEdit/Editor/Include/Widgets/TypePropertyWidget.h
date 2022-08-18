#pragma once

#include <QWidget>
#include <Types/QGlacierValue.h>


namespace widgets
{
	/*
	 * This widget representing inside data as one (or more) standard Qt widgets (QEdit, QLabel, Q...Matrix?)
	 */
	class TypePropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		TypePropertyWidget(QWidget* parent = nullptr);

		void setValue(const types::QGlacierValue &value);
		void updateValue(const types::QGlacierValue &value);
		[[nodiscard]] const types::QGlacierValue &getValue() const;

	signals:
		void valueChanged();

	private:
		void rebuildLayout();
		void clearLayout();

	private:
		types::QGlacierValue m_value {};
	};
}