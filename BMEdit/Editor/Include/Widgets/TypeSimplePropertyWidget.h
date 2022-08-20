#pragma once

#include <QStyleOptionViewItem>
#include <GameLib/PRP/PRPInstruction.h>
#include <Widgets/TypePropertyWidget.h>


namespace widgets
{
	class TypeSimplePropertyWidget : public TypePropertyWidget
	{
		Q_OBJECT

		using Base = TypePropertyWidget;
	public:
		using TypePropertyWidget::TypePropertyWidget;

		void setValue(const types::QGlacierValue &value) override;

		// Static
		static void paintTrivialView(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data);
	private:
		void buildLayout(const types::QGlacierValue &value);
		void updateLayout(const types::QGlacierValue &value);

		// Layout variations
		void createCheckBoxLayout(const types::QGlacierValue &value);
		void updateCheckBoxLayout(const types::QGlacierValue &value);

		void createNumberLayout(const types::QGlacierValue &value);
		void updateNumberLayout(const types::QGlacierValue &value);

		void createStringLayout(const types::QGlacierValue &value);
		void updateStringLayout(const types::QGlacierValue &value);

		void createEnumLayout(const types::QGlacierValue &value);
		void updateEnumLayout(const types::QGlacierValue &value);
	};
}