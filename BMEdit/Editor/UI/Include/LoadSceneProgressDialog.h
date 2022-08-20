#pragma once

#include <QDialog>

namespace Ui {
	class LoadSceneProgressDialog;
}

class LoadSceneProgressDialog : public QDialog
{
	Q_OBJECT

public:
	explicit LoadSceneProgressDialog(QWidget *parent = nullptr);
	~LoadSceneProgressDialog();

	void setLevelPath(const QString& levelPath);

private:
	void onLevelLoadError(const QString& error);
	void onLevelLoadProgress(int progress, const QString& operationName);
	void onLevelLoadSuccess();

private:
	Ui::LoadSceneProgressDialog *ui;
};