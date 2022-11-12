#pragma once

#include <QStyleOptionViewItem>
#include <GameLib/PRP/PRPInstruction.h>
#include <Widgets/TypePropertyWidget.h>


namespace widgets
{
	class TypeRefTabPropertyWidget : public TypePropertyWidget
	{
		Q_OBJECT

		using Base = TypePropertyWidget;
	public:
		using TypePropertyWidget::TypePropertyWidget;

		// Static
		static void paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data);
	private:
		void buildLayout(const types::QGlacierValue &value) override;
		void updateLayout(const types::QGlacierValue &value) override;

		// Layout variations
		void createLayout(const types::QGlacierValue &value);
		void createLayoutForEmptyContainer(const types::QGlacierValue &value);

		// Layout helpers
		QLayout* createLayoutForFooter();
		QLayout* createLayoutForEntry(const types::QGlacierValue &value, int instructionIndex);

	private slots:
		void showAddEntriesContextMenu();
		void addEntryFromContextMenu();
	};
}