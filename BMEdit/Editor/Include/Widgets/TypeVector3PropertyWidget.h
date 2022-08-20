#pragma once

#include <Widgets/TypePropertyWidget.h>


namespace widgets
{
	class TypeVector3PropertyWidget : public TypePropertyWidget
	{
		Q_OBJECT

		using Base = TypePropertyWidget;
	public:
		using TypePropertyWidget::TypePropertyWidget;

		void setValue(const types::QGlacierValue &value) override;

		// Static
		static void paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data);

	private:
		void buildLayout(const types::QGlacierValue &value);
		void updateLayout(const types::QGlacierValue &value);
	};
}