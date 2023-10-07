#include "ui_BMEditMainWindow.h"
#include "BMEditMainWindow.h"
#include "TypeViewerWindow.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringListModel>
#include <QClipboard>

#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeNotFoundException.h>

#include <Editor/EditorInstance.h>

#include <Models/SceneObjectsTreeModel.h>
#include <Models/SceneObjectPropertiesModel.h>
#include <Models/ScenePropertiesModel.h>
#include <Models/SceneFilterModel.h>
#include <Models/SceneTexturesModel.h>

#include <Delegates/TypePropertyItemDelegate.h>
#include <Delegates/ScenePropertyTypeDelegate.h>

#include <Widgets/GeomControllersWidget.h>
#include <Types/QCustomRoles.h>

#include <LoadSceneProgressDialog.h>

#include <nlohmann/json.hpp>


enum OperationToProgress : int
{
	DISCOVER_TYPES_DATABASE = 5,
	PARSE_DATABASE = 7,
	DATABASE_PARSED = 10,
	TYPE_DESCRIPTIONS_FOUND = 15,
	LOADING_TYPE_DESCRIPTORS = 20
};


BMEditMainWindow::BMEditMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BMEditMainWindow),
    m_loadSceneDialog(this),
	m_viewTexturesDialog(this)
{
    ui->setupUi(this);

	initSceneTree();
	initProperties();
	initControllers();
	initSceneProperties();
	initSceneLoadingDialog();
	initViewTexturesDialog();
	initStatusBar();
	initSearchInput();
	connectActions();
	connectDockWidgetActions();
	connectEditorSignals();
	loadTypesDataBase();
}

BMEditMainWindow::~BMEditMainWindow()
{
	delete m_geomTypesModel;
	delete m_typePropertyItemDelegate;
	delete m_sceneTreeFilterModel;
	delete m_sceneTreeModel;
	delete m_sceneObjectPropertiesModel;

	delete m_operationProgress;
	delete m_operationLabel;
	delete m_operationCommentLabel;
	delete ui;
}

void BMEditMainWindow::initStatusBar()
{
	m_operationProgress = new QProgressBar(statusBar());
	m_operationLabel = new QLabel(statusBar());
	m_operationCommentLabel = new QLabel(statusBar());

	resetStatusToDefault();

	statusBar()->insertWidget(0, m_operationLabel);
	statusBar()->insertWidget(1, m_operationProgress);
	statusBar()->insertWidget(2, m_operationCommentLabel);
}

void BMEditMainWindow::initSearchInput()
{
	connect(ui->searchInputField, &QLineEdit::textChanged, [=](const QString &query) { onSearchObjectQueryChanged(query); });
}

void BMEditMainWindow::connectActions()
{
	connect(ui->actionExit, &QAction::triggered, this, &BMEditMainWindow::onExit);
	connect(ui->actionOpen_level, &QAction::triggered, this, &BMEditMainWindow::onOpenLevel);
	connect(ui->actionRestore_layout, &QAction::triggered, this, &BMEditMainWindow::onRestoreLayout);
	connect(ui->actionTypes_Viewer, &QAction::triggered, this, &BMEditMainWindow::onShowTypesViewer);
	connect(ui->actionSave_properties, &QAction::triggered, this, &BMEditMainWindow::onExportProperties);
	connect(ui->actionExport_PRP_properties, &QAction::triggered, this, &BMEditMainWindow::onExportPRP);
	connect(ui->actionTextures, &QAction::triggered, this, &BMEditMainWindow::onShowTexturesDialog);
}

void BMEditMainWindow::connectDockWidgetActions()
{
	// Tools
	connect(ui->toolsDock, &QDockWidget::visibilityChanged, [=](bool visibility)
	{
		QSignalBlocker actionToolsBlocker{ui->actionTools};

		ui->actionTools->setChecked(visibility);
	});

	connect(ui->actionTools, &QAction::triggered, [=](bool checked)
	{
		QSignalBlocker toolsDockBlocker{ui->toolsDock};

		ui->toolsDock->setVisible(checked);
	});

	// Scene
	connect(ui->sceneDock, &QDockWidget::visibilityChanged, [=](bool visibility)
	{
		QSignalBlocker actionSceneBlocker{ui->actionScene};

		ui->actionScene->setChecked(visibility);
	});

	connect(ui->actionScene, &QAction::triggered, [=](bool checked)
	{
		QSignalBlocker sceneDockBlocker{ui->sceneDock};

		ui->sceneDock->setVisible(checked);
	});

	// Properties
	connect(ui->propertiesDock, &QDockWidget::visibilityChanged, [=](bool visibility)
	{
		QSignalBlocker actionPropertiesBlocker{ui->actionProperties};

		ui->actionProperties->setChecked(visibility);
	});

	connect(ui->actionProperties, &QAction::triggered, [=](bool checked)
	{
		QSignalBlocker propertiesDockBlocker{ui->propertiesDock};

		ui->propertiesDock->setVisible(checked);
	});
}

void BMEditMainWindow::connectEditorSignals()
{
	using editor::EditorInstance;

	auto &instance = EditorInstance::getInstance();

	connect(&instance, &EditorInstance::levelLoadSuccess, [=]() { onLevelLoadSuccess(); });
	connect(&instance, &EditorInstance::levelLoadFailed, [=](const QString &reason) { onLevelLoadFailed(reason); });
	connect(&instance, &EditorInstance::exportAssetSuccess, [=](gamelib::io::AssetKind assetKind, const QString &assetName) { onAssetExportedSuccessfully(assetKind, assetName); });
	connect(&instance, &EditorInstance::exportAssetFailed, [=](const QString &reason) { onAssetExportFailed(reason); });
}

void BMEditMainWindow::onExit()
{
	close();
}

void BMEditMainWindow::onOpenLevel()
{
	QFileDialog openLevelDialog(this, QString("Open Level"), QString(), QString("Hitman Blood Money level package (*.ZIP)"));
	openLevelDialog.setViewMode(QFileDialog::Detail);
	openLevelDialog.setFileMode(QFileDialog::ExistingFile);
	if (!openLevelDialog.exec())
	{
		return;
	}

	auto selectedLevel = openLevelDialog.selectedFiles().first().toStdString();
	auto &editorInstance = editor::EditorInstance::getInstance();

	//TODO: Restore code bellow after async loader will be finished
//	m_loadSceneDialog.setLevelPath(QString::fromStdString(selectedLevel));
//	m_loadSceneDialog.show();
	editor::EditorInstance::getInstance().openLevelFromZIP(selectedLevel);
}

void BMEditMainWindow::onRestoreLayout() {
	ui->propertiesDock->setVisible(true);
	ui->sceneDock->setVisible(true);
	ui->propertiesDock->setVisible(true);
}

void BMEditMainWindow::onShowTypesViewer()
{
	TypeViewerWindow viewerWindow(this);
	viewerWindow.setModal(true);
	viewerWindow.exec();
}

void BMEditMainWindow::onLevelLoadSuccess()
{
	auto currentLevel = editor::EditorInstance::getInstance().getActiveLevel();
	setWindowTitle(QString("BMEdit - %1 [loading view...]").arg(QString::fromStdString(currentLevel->getLevelName())));

	resetStatusToDefault();

	// Level loaded, show objects tree
	ui->searchInputField->clear();

	if (m_sceneTreeModel)
	{
		m_sceneTreeModel->setLevel(currentLevel);
	}

	if (m_sceneTexturesModel)
	{
		m_sceneTexturesModel->setLevel(currentLevel);
	}

	if (m_sceneObjectPropertiesModel)
	{
		m_sceneObjectPropertiesModel->setLevel(currentLevel);
	}

	if (m_scenePropertiesModel)
	{
		m_scenePropertiesModel->setLevel(const_cast<gamelib::Level*>(currentLevel));
	}

	// Show game scene (start loading process)
	ui->sceneGLView->setLevel(const_cast<gamelib::Level*>(currentLevel));

	// Load controllers index
	ui->geomControllers->switchToDefaults();

	// Export action
	ui->menuExport->setEnabled(true);
	ui->actionExport_PRP_properties->setEnabled(true);
	ui->actionTextures->setEnabled(true);

	//ui->actionSave_properties->setEnabled(true); //TODO: Uncomment when exporter to ZIP will be done
	ui->searchInputField->setEnabled(true);

	// Finished
	QApplication::beep();
}

void BMEditMainWindow::onLevelLoadFailed(const QString &reason)
{
	QMessageBox::warning(this, QString("Failed to load level"), QString("Error occurred during level load process:\n%1").arg(reason));
	m_operationCommentLabel->setText(QString("Failed to open level '%1'").arg(reason));
	m_operationProgress->setValue(0);
}

void BMEditMainWindow::onLevelLoadProgressChanged(int totalPercentsProgress, const QString &currentOperationTag)
{
	// Clamp value between [0, 100]
	totalPercentsProgress = std::min(totalPercentsProgress, 100);
	totalPercentsProgress = std::max(totalPercentsProgress, 0);

	// Update value if it greater
	if (m_operationProgress->value() < totalPercentsProgress)
	{
		m_operationProgress->setValue(totalPercentsProgress);
	}

	// Set operation tag
	if (!currentOperationTag.isEmpty())
	{
		m_operationCommentLabel->setText(QString("LOAD LEVEL: %1").arg(currentOperationTag));
	}
}

void BMEditMainWindow::onSearchObjectQueryChanged(const QString &query)
{
	if (m_sceneTreeFilterModel && m_sceneTreeFilterModel->getQuery() != query)
	{
		ui->sceneTreeView->collapseAll();
		m_sceneTreeFilterModel->setFilterKeyColumn(0);
		m_sceneTreeFilterModel->setQuery(query);
	}
}

void BMEditMainWindow::onSelectedSceneObject(const gamelib::scene::SceneObject* selectedSceneObject)
{
	if (!m_sceneObjectPropertiesModel || !selectedSceneObject)
	{
		return;
	}

	ui->sceneObjectName->setText(QString::fromStdString(selectedSceneObject->getName()));
	ui->sceneObjectTypeCombo->setEnabled(true);
	ui->sceneObjectTypeCombo->setCurrentText(QString::fromStdString(selectedSceneObject->getType()->getName()));

	m_sceneObjectPropertiesModel->setGeom(const_cast<gamelib::scene::SceneObject*>(selectedSceneObject));

	ui->geomControllers->setGeom(const_cast<gamelib::scene::SceneObject*>(selectedSceneObject));
	ui->geomControllers->switchToFirstController();
}

void BMEditMainWindow::onDeselectedSceneObject()
{
	if (!m_sceneObjectPropertiesModel)
	{
		return;
	}

	ui->sceneObjectTypeCombo->setEnabled(false);
	ui->sceneObjectName->clear();
	ui->geomControllers->resetGeom();

	m_sceneObjectPropertiesModel->resetGeom();
}

void BMEditMainWindow::onExportProperties()
{
	auto& editorInstance = editor::EditorInstance::getInstance();
	editorInstance.exportAsset(gamelib::io::AssetKind::PROPERTIES);
}

void BMEditMainWindow::onAssetExportedSuccessfully(gamelib::io::AssetKind assetKind, const QString &assetName)
{
	Q_UNUSED(assetKind);

	QMessageBox::information(this, QString("Export asset"), QString("Asset %1 exported successfully!").arg(assetName));
}

void BMEditMainWindow::onAssetExportFailed(const QString &reason)
{
	QMessageBox::critical(this, QString("Export asset"), QString("Unable to export asset: %1").arg(reason));
	m_operationCommentLabel->setText(QString("Asset export FAILED: %1").arg(reason));
}

void BMEditMainWindow::onCloseLevel()
{
	// Cleanup models
	if (m_sceneTreeModel) m_sceneTreeModel->resetLevel();
	if (m_sceneObjectPropertiesModel) m_sceneObjectPropertiesModel->resetLevel();
	if (m_scenePropertiesModel) m_scenePropertiesModel->resetLevel();
	if (m_sceneTexturesModel) m_sceneTexturesModel->resetLevel();

	// Unload resources
	ui->sceneGLView->resetLevel();

	// Reset widget states
	ui->geomControllers->resetGeom();

	// Reset export menu
	ui->menuExport->setEnabled(false);
	ui->actionExport_PRP_properties->setEnabled(false);
	ui->actionTextures->setEnabled(false);

	// Disable filtering
	QSignalBlocker blocker { ui->searchInputField };
	ui->searchInputField->setEnabled(false);
	ui->searchInputField->clear();
}

void BMEditMainWindow::onExportPRP()
{
	QFileDialog savePRPDialog(this, QString("Save PRP"), QString(), QString("Scene properties (*.PRP)"));
	savePRPDialog.setViewMode(QFileDialog::ViewMode::Detail);
	savePRPDialog.setFileMode(QFileDialog::FileMode::AnyFile);
	savePRPDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	savePRPDialog.selectFile(QString("%1.PRP").arg(QString::fromStdString(editor::EditorInstance::getInstance().getActiveLevel()->getLevelName())));
	if (!savePRPDialog.exec())
	{
		return;
	}

	if (savePRPDialog.selectedFiles().empty())
	{
		return;
	}

	const auto saveAsPath = savePRPDialog.selectedFiles().first();
	editor::EditorInstance::getInstance().exportPRP(saveAsPath);

	QMessageBox::information(this, "Export PRP", QString("PRP file exported successfully to %1").arg(saveAsPath));
}

void BMEditMainWindow::onShowTexturesDialog()
{
	m_viewTexturesDialog.show();
}

void BMEditMainWindow::onContextMenuRequestedForSceneTreeNode(const QPoint& point)
{
	if (!m_sceneTreeModel)
	{
		return;
	}

	QModelIndex index = ui->sceneTreeView->indexAt(point);
	if (!index.isValid())
	{
		return;
	}

	QModelIndex mappedIndex = m_sceneTreeFilterModel->mapToSource(index);
	const auto* selectedGeom = reinterpret_cast<const gamelib::scene::SceneObject*>(mappedIndex.data(types::kSceneObjectRole).value<std::intptr_t>());

	if (selectedGeom)
	{
		QMenu contextMenu;

		auto implCopyPathToGeom = [](const gamelib::scene::SceneObject* sceneObject, bool ignoreRoot)
		{
			QStringList pathEntries;

			const gamelib::scene::SceneObject* currentObject = sceneObject;
			while (currentObject)
			{
				pathEntries.push_front(QString::fromStdString(currentObject->getName()));
				const auto& sp = currentObject->getParent().lock();
				currentObject = sp ? sp.get() : nullptr;

				if (currentObject && currentObject->getParent().expired() && ignoreRoot)
					break;
			}

			auto finalPath = pathEntries.join('\\');
			QGuiApplication::clipboard()->setText(finalPath);
		};

		auto implMoveCameraToGeom = [this](gamelib::scene::SceneObject* sceneObject)
		{
			const glm::vec3 vPosition {
			    sceneObject->getProperties()["Position"][1].getOperand().get<float>(),
			    sceneObject->getProperties()["Position"][2].getOperand().get<float>(),
			    sceneObject->getProperties()["Position"][3].getOperand().get<float>()
			};

			ui->sceneGLView->moveCameraTo(vPosition);
		};

		contextMenu.addAction(QString("Object: '%1'").arg(QString::fromStdString(selectedGeom->getName())))->setDisabled(true);
		contextMenu.addAction(QString("Type: '%1'").arg(QString::fromStdString(selectedGeom->getType()->getName())))->setDisabled(true);
		contextMenu.addSeparator();
		contextMenu.addAction("Copy path", [implCopyPathToGeom, selectedGeom] { implCopyPathToGeom(selectedGeom, false); });
		contextMenu.addAction("Copy path (ignore ROOT)", [implCopyPathToGeom, selectedGeom] { implCopyPathToGeom(selectedGeom, true); });
		contextMenu.addAction("Move camera to this object", [implMoveCameraToGeom, selectedGeom] { implMoveCameraToGeom(const_cast<gamelib::scene::SceneObject*>(selectedGeom)); });

		contextMenu.exec(ui->sceneTreeView->viewport()->mapToGlobal(point));
	}
}

void BMEditMainWindow::onLevelAssetsLoaded()
{
	auto currentLevel = editor::EditorInstance::getInstance().getActiveLevel();
	setWindowTitle(QString("BMEdit - %1 [DONE]").arg(QString::fromStdString(currentLevel->getLevelName())));
}

void BMEditMainWindow::onLevelAssetsLoadFailed(const QString& reason)
{
	auto currentLevel = editor::EditorInstance::getInstance().getActiveLevel();
	setWindowTitle(QString("BMEdit - %1 [!!!ERROR!!!]").arg(QString::fromStdString(currentLevel->getLevelName())));

	QMessageBox::critical(this, QString("Scene render failed :("), QString("An error occurred while loading scene assets:\n%1").arg(reason));
}

void BMEditMainWindow::loadTypesDataBase()
{
	m_operationProgress->setValue(OperationToProgress::DISCOVER_TYPES_DATABASE);

	gamelib::TypeRegistry::getInstance().reset();

	QFile typeRegistryFile("TypesRegistry.json");
	if (!typeRegistryFile.open(QIODevice::ReadOnly))
	{
		m_operationCommentLabel->setText("Load 'TypesRegistry.json' failed. File not found");
		return;
	}

	m_operationProgress->setValue(OperationToProgress::PARSE_DATABASE);
	auto contents = typeRegistryFile.readAll().toStdString();
	typeRegistryFile.close();

	auto registryFile = nlohmann::json::parse(contents, nullptr, false, true);
	if (registryFile.is_discarded()) {
		m_operationCommentLabel->setText("Failed to load types database: invalid JSON format");
		return;
	}

	m_operationProgress->setValue(OperationToProgress::DATABASE_PARSED);

	if (!registryFile.contains("inc") || !registryFile.contains("db"))
	{
		m_operationCommentLabel->setText("Invalid types database format");
		return;
	}

	m_operationProgress->setValue(OperationToProgress::TYPE_DESCRIPTIONS_FOUND);
	auto &registry = gamelib::TypeRegistry::getInstance();

	std::unordered_map<std::string, std::string> typesToHashes;
	for (const auto &[hash, typeNameObj]: registryFile["db"].items())
	{
		typesToHashes[typeNameObj.get<std::string>()] = hash;
	}

	const auto incPath = registryFile["inc"].get<std::string>();

	m_operationProgress->setValue(OperationToProgress::LOADING_TYPE_DESCRIPTORS);
	m_operationCommentLabel->setText(QString("Hash indices loaded (%1), loading types from '%2' folder").arg(typesToHashes.size()).arg(QString::fromStdString(incPath)));

	// Here we need to scan for all .json files in 'inc' folder and parse 'em all
	std::vector<nlohmann::json> typeInfos;
	QDirIterator typesFolderIterator(QString::fromStdString(incPath), { "*.json" }, QDir::Files);
	while (typesFolderIterator.hasNext())
	{
		auto path = typesFolderIterator.next();

		// TODO: It's better to do in multiple threads but who cares?)
		QFile typeDescriptionFile(path);
		if (!typeDescriptionFile.open(QIODevice::ReadOnly))
		{
			m_operationCommentLabel->setText(QString("ERROR: Failed to open file '%1'").arg(path));
			return;
		}

		auto typeInfoContents = typeDescriptionFile.readAll().toStdString();
		typeDescriptionFile.close();


		auto &jsonContents = typeInfos.emplace_back();
		jsonContents = nlohmann::json::parse(typeInfoContents, nullptr, false, true);
		if (jsonContents.is_discarded())
		{
			m_operationCommentLabel->setText(QString("ERROR: Failed to parse file '%1'").arg(path));
			return;
		}
	}

	try
	{
		registry.registerTypes(std::move(typeInfos), std::move(typesToHashes));

		QStringList allAvailableTypes;
		gamelib::TypeRegistry::getInstance().forEachType([&allAvailableTypes](const gamelib::Type *type) { allAvailableTypes.push_back(QString::fromStdString(type->getName())); });

		delete m_geomTypesModel;
		m_geomTypesModel = new QStringListModel(allAvailableTypes, this);
		ui->sceneObjectTypeCombo->setModel(m_geomTypesModel);

		m_operationProgress->setValue(0);
		m_operationCommentLabel->setText("Ready to open level");
	}
	catch (const gamelib::TypeNotFoundException &typeNotFoundException)
	{
		m_operationCommentLabel->setText(QString("ERROR: Unable to load types database: %1").arg(QString::fromStdString(typeNotFoundException.what())));
		QMessageBox::critical(this, QString("Unable to load types database"), QString("An error occurred while loading types database:\n%1").arg(typeNotFoundException.what()));
	}
	catch (const std::exception &somethingGoesWrong)
	{
		m_operationCommentLabel->setText(QString("ERROR: Unknown exception in type loader: %1").arg(QString::fromStdString(somethingGoesWrong.what())));
		QMessageBox::critical(this, QString("Unable to load types database"), QString("An error occurred while loading types database:\n%1").arg(somethingGoesWrong.what()));
	}
}

void BMEditMainWindow::resetStatusToDefault()
{
	m_operationLabel->setText("Progress: ");
	m_operationCommentLabel->setText("(No active operation)");
	m_operationProgress->setValue(0);
}

void BMEditMainWindow::initSceneTree()
{
	// Main model
	m_sceneTreeModel = new models::SceneObjectsTreeModel(this);
	m_sceneTreeFilterModel = new models::SceneFilterModel(this);
	m_sceneTreeFilterModel->setSourceModel(m_sceneTreeModel);

	ui->sceneTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
	ui->sceneTreeView->setModel(m_sceneTreeFilterModel);
	ui->sceneTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->sceneTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
		const bool somethingSelected = !selected.indexes().isEmpty();

		if (somethingSelected) {
			QModelIndex mappedSelection = m_sceneTreeFilterModel->mapToSource(selected.indexes().first());

			const auto* selectedGeom = reinterpret_cast<const gamelib::scene::SceneObject*>(mappedSelection.data(types::kSceneObjectRole).value<std::intptr_t>());

			if (selectedGeom)
			{
				onSelectedSceneObject(selectedGeom);
			}
		} else if (!deselected.indexes().isEmpty())
		{
			onDeselectedSceneObject();
		}
	});

	connect(ui->sceneTreeView, &QTreeView::customContextMenuRequested, this, &BMEditMainWindow::onContextMenuRequestedForSceneTreeNode);

	connect(ui->sceneGLView, &widgets::SceneRenderWidget::resourcesReady, this, &BMEditMainWindow::onLevelAssetsLoaded);
	connect(ui->sceneGLView, &widgets::SceneRenderWidget::resourceLoadFailed, this, &BMEditMainWindow::onLevelAssetsLoadFailed);
}

void BMEditMainWindow::initProperties()
{
	m_sceneObjectPropertiesModel = new models::SceneObjectPropertiesModel(this);
	m_typePropertyItemDelegate = new delegates::TypePropertyItemDelegate(this);

	ui->propertiesView->setModel(m_sceneObjectPropertiesModel);
	ui->propertiesView->setItemDelegateForColumn(1, m_typePropertyItemDelegate);
	ui->propertiesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->propertiesView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

void BMEditMainWindow::initSceneProperties()
{
	m_scenePropertiesModel = new models::ScenePropertiesModel(this);
	m_scenePropertyItemDelegate = new delegates::ScenePropertyTypeDelegate(this);

	ui->sceneProperties->setModel(m_scenePropertiesModel);
	ui->sceneProperties->setItemDelegateForColumn(1, m_scenePropertyItemDelegate);

	ui->sceneProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
	ui->sceneProperties->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
}

void BMEditMainWindow::initControllers()
{
	//TODO: Init this
}

void BMEditMainWindow::initSceneLoadingDialog()
{
	m_loadSceneDialog.setFixedSize(m_loadSceneDialog.size());
	m_loadSceneDialog.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	m_loadSceneDialog.setModal(true);
}

void BMEditMainWindow::initViewTexturesDialog()
{
	m_sceneTexturesModel.reset(new models::SceneTexturesModel(this));
	m_viewTexturesDialog.setModal(true);
	m_viewTexturesDialog.setTexturesSource(m_sceneTexturesModel.get());
}