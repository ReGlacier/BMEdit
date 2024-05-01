#pragma once

#include <Widgets/TypePropertyWidget.h>
#include <QCloseEvent>
#include <QLabel>


class Frontend_SelectSceneObjectTool final : public widgets::TypePropertyWidget
{
	Q_OBJECT
public:
	Frontend_SelectSceneObjectTool(QWidget* parent, widgets::TypePropertyWidget* pTarget);
	~Frontend_SelectSceneObjectTool() override;

	void setValue(const types::QGlacierValue &value) override;
	[[nodiscard]] const types::QGlacierValue &getValue() const override;

	bool canHookFocus() const override;

private:
	void commitValue(const types::QGlacierValue &value);

private slots:
	void onTargetValueChanged();
	void onTargetEditFinished();

private:
	widgets::TypePropertyWidget* m_pTarget { nullptr };
	QLabel* m_pLabel { nullptr };
};


namespace Ui {
	class SelectSceneObjectTool;
}

class SelectSceneObjectTool final : public widgets::TypePropertyWidget
{
	Q_OBJECT
public:
	explicit SelectSceneObjectTool(QWidget* parent = nullptr);
	~SelectSceneObjectTool() override;

	void setValue(const types::QGlacierValue &value) override;

	static Frontend_SelectSceneObjectTool* Create(QWidget* parent);

private:
	void selectByPath(const QString& path);

private slots:
	void onAccepted();

protected:
	// buildLayout and updateLayout not implemented because no dynamic layout here. I'm just handling setValue
	void closeEvent(QCloseEvent* pEvent) override;

private:
	Ui::SelectSceneObjectTool *m_ui;
};