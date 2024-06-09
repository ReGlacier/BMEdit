#pragma once

#include <Widgets/TypePropertyWidget.h>
#include <QItemSelection>
#include <QCloseEvent>
#include <QLabel>


class Frontend_SelectLocalizationTool final : public widgets::TypePropertyWidget
{
	Q_OBJECT
public:
	Frontend_SelectLocalizationTool(QWidget* parent, widgets::TypePropertyWidget* pTarget);
	~Frontend_SelectLocalizationTool() override;

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
	class SelectLocalizationTool;
}

class SelectLocalizationTool : public widgets::TypePropertyWidget
{
	Q_OBJECT

public:
	explicit SelectLocalizationTool(QWidget *parent = nullptr);
	~SelectLocalizationTool();

	static Frontend_SelectLocalizationTool* Create(QWidget* parent);

	void setValue(const types::QGlacierValue &value) override;

protected:
	// buildLayout and updateLayout not implemented because no dynamic layout here. I'm just handling setValue
	void closeEvent(QCloseEvent* pEvent) override;

private:
	void disableAcceptButton();
	void enableAcceptButton();
	void selectByPath(const QString& path);

private slots:
	void onAccepted();
	void onRejected();
	void onLocaleSelected(const QItemSelection &selected, const QItemSelection &deselected);

private:
	Ui::SelectLocalizationTool *m_ui;
};