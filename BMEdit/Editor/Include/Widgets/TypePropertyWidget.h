#pragma once

#include <QWidget>
#include <QStyleOptionViewItem>
#include <Types/QGlacierValue.h>


namespace widgets
{
	/*
	 * @class TypePropertyWidget
	 * @brief Base class for 'editor' widgets
	 */
	class TypePropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		explicit TypePropertyWidget(QWidget* parent = nullptr);

		virtual void setValue(const types::QGlacierValue &value);
		[[nodiscard]] const types::QGlacierValue &getValue() const;

	signals:
		void valueChanged();
		void editFinished();

	protected:
		types::QGlacierValue m_value {};
	};
}