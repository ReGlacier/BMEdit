#pragma once

#include <Widgets/TypePropertyWidget.h>
#include <QItemSelection>
#include <QCloseEvent>
#include <QLabel>


class Frontend_SelectScriptTool final : public widgets::TypePropertyWidget
{
	Q_OBJECT
public:
	Frontend_SelectScriptTool(QWidget* parent, widgets::TypePropertyWidget* pTarget);
	~Frontend_SelectScriptTool() override;

	void setValue(const types::QGlacierValue &value) override;
	[[nodiscard]] const types::QGlacierValue &getValue() const override;

	bool canHookFocus() const override;

private slots:
	void onTargetValueChanged();
	void onTargetEditFinished();

private:
	void commitValue(const types::QGlacierValue &value);

private:
	widgets::TypePropertyWidget* m_pTarget { nullptr };
	QLabel* m_pLabel { nullptr };
};


namespace Ui {
	class SelectScriptTool;
}

class SelectScriptTool : public widgets::TypePropertyWidget
{
	Q_OBJECT

public:
	explicit SelectScriptTool(QWidget *parent = nullptr);
	~SelectScriptTool();

	static Frontend_SelectScriptTool* Create(QWidget* parent);

protected:
	// buildLayout and updateLayout not implemented because no dynamic layout here. I'm just handling setValue
	void closeEvent(QCloseEvent* pEvent) override;

private:
	void disableAcceptButton();
	void enableAcceptButton();

private slots:
	void onAccepted();
	void onRejected();
	void onScriptSelected(const QItemSelection &selected, const QItemSelection &deselected);

private:
	Ui::SelectScriptTool *m_ui;
};