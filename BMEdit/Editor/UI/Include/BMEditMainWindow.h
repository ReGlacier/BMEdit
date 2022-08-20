#ifndef BMEDITMAINWINDOW_H
#define BMEDITMAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QString>
#include <QStringListModel>

#include "LoadSceneProgressDialog.h"


namespace Ui {
class BMEditMainWindow;
}

namespace models
{
	class SceneObjectPropertiesModel;
	class SceneObjectsTreeModel;
}

namespace delegates
{
	class TypePropertyItemDelegate;
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
	void connectActions();
	void connectDockWidgetActions();
	void connectEditorSignals();
	void loadTypesDataBase();
	void resetStatusToDefault();
	void initSceneTree();
	void initProperties();
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

	// Delegates
	delegates::TypePropertyItemDelegate *m_typePropertyItemDelegate { nullptr };

	// Dialogs
	LoadSceneProgressDialog m_loadSceneDialog;
};

#endif // BMEDITMAINWINDOW_H
