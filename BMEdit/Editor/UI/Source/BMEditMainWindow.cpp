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

#include <GTIL/GlacierTypeInfoLoader.h>

#include <Editor/EditorInstance.h>

#include <Models/SceneObjectsTreeModel.h>
#include <Models/SceneObjectPropertiesModel.h>
#include <Models/ScenePropertiesModel.h>
#include <Models/ScenePrimitivesModel.h>
#include <Models/ScenePrimitivesFilterModel.h>

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
    m_loadSceneDialog(this)
{
    ui->setupUi(this);

	initSceneTree();
	initProperties();
	initControllers();
	initSceneProperties();
	initScenePrimitives();
	initSceneLoadingDialog();
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
	delete m_sceneTreeModel;
	delete m_sceneObjectPropertiesModel;
	delete m_scenePrimitivesModel;

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
	connect(ui->actionExit, &QAction::triggered, [=]() { onExit(); });
	connect(ui->actionOpen_level, &QAction::triggered, [=]() { onOpenLevel(); });
	connect(ui->actionRestore_layout, &QAction::triggered, [=]() { onRestoreLayout(); });
	connect(ui->actionTypes_Viewer, &QAction::triggered, [=]() { onShowTypesViewer(); });
	connect(ui->actionSave_properties, &QAction::triggered, [=]() { onExportProperties(); });
	connect(ui->actionExport_PRP_properties, &QAction::triggered, [=]() { onExportPRP(); });
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
	setWindowTitle(QString("BMEdit - %1").arg(QString::fromStdString(currentLevel->getLevelName())));

	resetStatusToDefault();

	// Level loaded, show objects tree
	{
		QSignalBlocker blocker { ui->searchInputField };
		ui->searchInputField->clear();
	}

	if (m_sceneTreeModel)
	{
		m_sceneTreeModel->setLevel(currentLevel);
	}

	if (m_sceneObjectPropertiesModel)
	{
		m_sceneObjectPropertiesModel->setLevel(currentLevel);
	}

	if (m_scenePropertiesModel)
	{
		m_scenePropertiesModel->setLevel(const_cast<gamelib::Level*>(currentLevel));
	}

	if (m_scenePrimitivesModel)
	{
		m_scenePrimitivesModel->setLevel(const_cast<gamelib::Level*>(currentLevel));
		ui->primitivesCountLabel->setText(QString("%1").arg(currentLevel->getLevelGeometry()->header.countOfPrimitives));
	}

	// Reset primitive filters
	resetPrimitivesFilter();

	// Setup preview
	ui->scenePrimitivePreview->setLevel(const_cast<gamelib::Level*>(currentLevel));

	// Load controllers index
	ui->geomControllers->switchToDefaults();

	// Export action
	ui->menuExport->setEnabled(true);
	ui->actionExport_PRP_properties->setEnabled(true);

	ui->searchInputField->clear();
	//ui->actionSave_properties->setEnabled(true); //TODO: Uncomment when exporter to ZIP will be done
	//ui->searchInputField->setEnabled(true); //TODO: Uncomment when search will be done

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
	if (m_scenePrimitivesModel) m_scenePrimitivesModel->resetLevel();

	// Reset filters
	resetPrimitivesFilter();

	// Reset
	ui->scenePrimitivePreview->resetLevel();

	// Reset widget states
	ui->geomControllers->resetGeom();

	// Reset export menu
	ui->menuExport->setEnabled(false);
	ui->actionExport_PRP_properties->setEnabled(false);

	// Reset primitives counter
	ui->primitivesCountLabel->setText("0");
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

	const auto* selectedGeom = reinterpret_cast<const gamelib::scene::SceneObject*>(index.data(types::kSceneObjectRole).value<std::intptr_t>());

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

		contextMenu.addAction(QString("Object: '%1'").arg(QString::fromStdString(selectedGeom->getName())))->setDisabled(true);
		contextMenu.addAction(QString("Type: '%1'").arg(QString::fromStdString(selectedGeom->getType()->getName())))->setDisabled(true);
		contextMenu.addSeparator();
		contextMenu.addAction("Copy path", [implCopyPathToGeom, selectedGeom] { implCopyPathToGeom(selectedGeom, false); });
		contextMenu.addAction("Copy path (ignore ROOT)", [implCopyPathToGeom, selectedGeom] { implCopyPathToGeom(selectedGeom, true); });

		contextMenu.exec(ui->sceneTreeView->viewport()->mapToGlobal(point));
	}
}

void BMEditMainWindow::onContextMenuRequestedForPrimitivesTableHeader(const QPoint &point)
{
	QMenu contextMenu;

#define BE_CONFIGURE_ACTION(actName, actFmt) \
	{ \
		auto action = contextMenu.addAction(actName);  \
		action->setCheckable(true);  \
		action->setChecked(m_scenePrimitivesFilterModel->isVertexFormatAllowed(actFmt)); \
        connect(action, &QAction::toggled, [this](bool val) { m_scenePrimitivesFilterModel->setVertexFormatAllowed(actFmt, val); }); \
	}

	BE_CONFIGURE_ACTION("Vertex Format 10", gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_10);
	BE_CONFIGURE_ACTION("Vertex Format 24", gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_24);
	BE_CONFIGURE_ACTION("Vertex Format 28", gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_28);
	BE_CONFIGURE_ACTION("Vertex Format 34", gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_34);

	contextMenu.exec(ui->scenePrimitivesTable->horizontalHeader()->viewport()->mapToGlobal(point));
}

void BMEditMainWindow::loadTypesDataBase()
{
	m_operationProgress->setValue(OperationToProgress::DISCOVER_TYPES_DATABASE);

	const auto& [errorCode, errorMessage] = gtil::GlacierTypeInfoLoader::loadTypes(std::filesystem::current_path() / "TypesRegistry.json");

	if (errorCode == gtil::ErrorCode::EC_NO_ERROR)
	{
		QStringList allAvailableTypes;
		gamelib::TypeRegistry::getInstance().forEachType([&allAvailableTypes](const gamelib::Type *type) { allAvailableTypes.push_back(QString::fromStdString(type->getName())); });

		delete m_geomTypesModel;
		m_geomTypesModel = new QStringListModel(allAvailableTypes, this);

		ui->sceneObjectTypeCombo->setModel(m_geomTypesModel);

		m_operationProgress->setValue(0);
		m_operationCommentLabel->setText("Ready to open level");
	}
	else
	{
		m_operationProgress->setValue(20);
		m_operationCommentLabel->setText(QString("Failed to load type database: %1").arg(QString::fromStdString(errorMessage)));

		QMessageBox::critical(this, QString("Unable to load types database"), QString("An error occurred while loading types database:\n%1").arg(QString::fromStdString(errorMessage)));
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
	ui->sceneTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
	ui->sceneTreeView->setModel(m_sceneTreeModel);
	ui->sceneTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->sceneTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
		const bool somethingSelected = !selected.indexes().isEmpty();

		if (somethingSelected) {
			const auto* selectedGeom = reinterpret_cast<const gamelib::scene::SceneObject*>(selected.indexes().first().data(types::kSceneObjectRole).value<std::intptr_t>());

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

void BMEditMainWindow::initScenePrimitives()
{
	m_scenePrimitivesModel = new models::ScenePrimitivesModel(this);
	m_scenePrimitivesFilterModel = new models::ScenePrimitivesFilterModel(this);

	m_scenePrimitivesFilterModel->setSourceModel(m_scenePrimitivesModel);
	ui->scenePrimitivesTable->setModel(m_scenePrimitivesFilterModel);
	ui->scenePrimitivesTable->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui->scenePrimitivesTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->scenePrimitivesTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->scenePrimitivesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

	connect(ui->scenePrimitivesTable->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
		if ((selected.indexes().size() == 1 && selected.indexes().at(0).row() != 0) || (!selected.indexes().empty()))
		{
			m_selectedPrimitiveToPreview.emplace(static_cast<std::uint32_t>(selected.indexes().first().data(types::kChunkIndexRole).value<int>()));
			ui->scenePrimitivePreview->setPrimitiveIndex(*m_selectedPrimitiveToPreview);
		}
		else if (!deselected.indexes().isEmpty())
		{
			m_selectedPrimitiveToPreview = std::nullopt;
		}

		ui->exportChunk->setEnabled(m_selectedPrimitiveToPreview.has_value());
	});

	connect(ui->exportChunk, &QPushButton::clicked, [=]() {
		if (m_selectedPrimitiveToPreview.has_value())
		{
			//TODO: Impl me
		}
	});

	connect(ui->scenePrimitivesTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &BMEditMainWindow::onContextMenuRequestedForPrimitivesTableHeader);

	auto sendChangesToFilterModel = [=](models::ScenePrimitivesFilterEntry entry, int newState)
	{
		if (newState)
		{
			m_scenePrimitivesFilterModel->addFilterEntry(entry);
		}
		else
		{
			m_scenePrimitivesFilterModel->removeFilterEntry(entry);
		}
	};

	connect(ui->primitivesFilter_UnknownPrimType,      &QCheckBox::stateChanged, [=](int newState) { sendChangesToFilterModel(models::ScenePrimitivesFilterEntry::FilterAllow_Unknown,     newState); });
	connect(ui->primitivesFilter_ZeroBufferPrimType,   &QCheckBox::stateChanged, [=](int newState) { sendChangesToFilterModel(models::ScenePrimitivesFilterEntry::FilterAllow_Zero,        newState); });
	connect(ui->primitivesFilter_DescriptionPrimType,  &QCheckBox::stateChanged, [=](int newState) { sendChangesToFilterModel(models::ScenePrimitivesFilterEntry::FilterAllow_Description, newState); });
	connect(ui->primitivesFilter_IndexBufferPrimType,  &QCheckBox::stateChanged, [=](int newState) { sendChangesToFilterModel(models::ScenePrimitivesFilterEntry::FilterAllow_Index,       newState); });
	connect(ui->primitivesFilter_VertexBufferPrimType, &QCheckBox::stateChanged, [=](int newState) { sendChangesToFilterModel(models::ScenePrimitivesFilterEntry::FilterAllow_Vertex,      newState); });
}

void BMEditMainWindow::initSceneLoadingDialog()
{
	m_loadSceneDialog.setFixedSize(m_loadSceneDialog.size());
	m_loadSceneDialog.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	m_loadSceneDialog.setModal(true);
}

void BMEditMainWindow::resetPrimitivesFilter()
{
	if (m_scenePrimitivesFilterModel)
	{
		m_scenePrimitivesFilterModel->resetToDefaults();
	}

#define BE_RESET_CHECK_BOX(x)  \
	{                          \
		QSignalBlocker blk{x}; \
		x->setChecked(true);   \
	}

	BE_RESET_CHECK_BOX(ui->primitivesFilter_UnknownPrimType);
	BE_RESET_CHECK_BOX(ui->primitivesFilter_ZeroBufferPrimType);
	BE_RESET_CHECK_BOX(ui->primitivesFilter_DescriptionPrimType);
	BE_RESET_CHECK_BOX(ui->primitivesFilter_IndexBufferPrimType);
	BE_RESET_CHECK_BOX(ui->primitivesFilter_VertexBufferPrimType);

#undef BE_RESET_CHECK_BOX
}