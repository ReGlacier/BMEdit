#include "LoadSceneProgressDialog.h"
#include "ui_LoadSceneProgressDialog.h"

#include <Editor/EditorInstance.h>


LoadSceneProgressDialog::LoadSceneProgressDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoadSceneProgressDialog)
{
	ui->setupUi(this);

	auto& editorInstance = editor::EditorInstance::getInstance();

	connect(&editorInstance, &editor::EditorInstance::levelLoadFailed, this, &LoadSceneProgressDialog::onLevelLoadError);
	connect(&editorInstance, &editor::EditorInstance::levelLoadProgressChanged, this, &LoadSceneProgressDialog::onLevelLoadProgress);
	connect(&editorInstance, &editor::EditorInstance::levelLoadSuccess, this, &LoadSceneProgressDialog::onLevelLoadSuccess);
}

LoadSceneProgressDialog::~LoadSceneProgressDialog()
{
	delete ui;
}

void LoadSceneProgressDialog::setLevelPath(const QString &levelPath)
{
	ui->scenePath->setText(levelPath);
	ui->doneButton->setDisabled(true);
	ui->loadProgress->setValue(0);

	//TODO: Set model
}

void LoadSceneProgressDialog::onLevelLoadError(const QString &error)
{
}

void LoadSceneProgressDialog::onLevelLoadProgress(int progress, const QString &operationName)
{
}

void LoadSceneProgressDialog::onLevelLoadSuccess()
{
}