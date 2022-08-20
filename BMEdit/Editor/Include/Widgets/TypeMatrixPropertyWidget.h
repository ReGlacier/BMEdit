#pragma once

#include <Widgets/TypePropertyWidget.h>


namespace widgets
{
	class TypeMatrixPropertyWidget : public TypePropertyWidget
	{
		Q_OBJECT

		using Base = TypePropertyWidget;
	public:
		TypeMatrixPropertyWidget(int rows, int columns, QWidget *parent = nullptr);

		// Static
		static void paintPreview(int rows, int columns, QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data);

	private:
		void buildLayout(const types::QGlacierValue &value) override;
		void updateLayout(const types::QGlacierValue &value) override;

	private:
		int m_rows { 0 };
		int m_columns { 0 };
	};
}