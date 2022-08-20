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

		void setValue(const types::QGlacierValue &value);
		[[nodiscard]] const types::QGlacierValue &getValue() const;

	signals:
		void valueChanged();
		void editFinished();

	protected:
		static bool areSame(const types::QGlacierValue &current, const types::QGlacierValue &value);

		virtual void buildLayout(const types::QGlacierValue &value);
		virtual void updateLayout(const types::QGlacierValue &value);

	protected:
		types::QGlacierValue m_value {};
	};
}