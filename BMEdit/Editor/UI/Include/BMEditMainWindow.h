#ifndef BMEDITMAINWINDOW_H
#define BMEDITMAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QString>
#include <QStringListModel>

#include <GameLib/IO/AssetKind.h>

#include "LoadSceneProgressDialog.h"


namespace Ui {
class BMEditMainWindow;
}

namespace models
{
	class SceneObjectPropertiesModel;
	class SceneObjectsTreeModel;
	class ScenePropertiesModel;
}

namespace delegates
{
	class TypePropertyItemDelegate;
	class ScenePropertyTypeDelegate;
}

namespace gamelib::scene
{
	class SceneObject;
}

class BMEditMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BMEditMainWindow(QWidget *parent = nullptr);
    ~BMEditMainWindow();

private:
	void initStatusBar();
	void initSearchInput();
	void connectActions();
	void connectDockWidgetActions();
	void connectEditorSignals();
	void loadTypesDataBase();
	void resetStatusToDefault();
	void initSceneTree();
	void initProperties();
	void initSceneProperties();
	void initControllers();
	void initSceneLoadingDialog();


public slots:
	void onExit();
	void onOpenLevel();
	void onRestoreLayout();
	void onShowTypesViewer();
	void onLevelLoadSuccess();
	void onLevelLoadFailed(const QString &reason);
	void onLevelLoadProgressChanged(int totalPercentsProgress, const QString &currentOperationTag);
	void onSearchObjectQueryChanged(const QString &query);
	void onSelectedSceneObject(const gamelib::scene::SceneObject* selectedSceneObject);
	void onDeselectedSceneObject();
	void onExportProperties();
	void onAssetExportedSuccessfully(gamelib::io::AssetKind assetKind, const QString &assetName);
	void onAssetExportFailed(const QString &reason);
	void onCloseLevel();
	void onExportPRP();

private:
    // UI
	Ui::BMEditMainWindow *ui;

    // Custom
    QLabel* m_operationLabel;
    QLabel* m_operationCommentLabel;
    QProgressBar* m_operationProgress;

	// Models
	QStringListModel *m_geomTypesModel { nullptr };
	models::SceneObjectsTreeModel *m_sceneTreeModel { nullptr };
	models::SceneObjectPropertiesModel *m_sceneObjectPropertiesModel { nullptr };
	models::ScenePropertiesModel *m_scenePropertiesModel { nullptr };

	// Delegates
	delegates::TypePropertyItemDelegate *m_typePropertyItemDelegate { nullptr };
	delegates::ScenePropertyTypeDelegate *m_scenePropertyItemDelegate { nullptr };

	// Dialogs
	LoadSceneProgressDialog m_loadSceneDialog;
};

#endif // BMEDITMAINWINDOW_H
