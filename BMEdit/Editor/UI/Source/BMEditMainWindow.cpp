#include "ui_BMEditMainWindow.h"
#include "BMEditMainWindow.h"
#include "TypeViewerWindow.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringListModel>

#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeNotFoundException.h>

#include <Editor/EditorInstance.h>
#include <Models/SceneObjectsTreeModel.h>
#include <Models/SceneObjectPropertiesModel.h>
#include <Delegates/TypePropertyItemDelegate.h>

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
    ui(new Ui::BMEditMainWindow)
{
    ui->setupUi(this);

    ui->sceneTreeView->setModel(new models::SceneObjectsTreeModel(this));
	ui->propertiesView->setModel(new models::SceneObjectPropertiesModel(this));
	ui->propertiesView->setItemDelegateForColumn(1, new delegates::TypePropertyItemDelegate(this));

	initStatusBar();
	connectActions();
	connectDockWidgetActions();
	connectEditorSignals();
	loadTypesDataBase();
}

BMEditMainWindow::~BMEditMainWindow()
{
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

void BMEditMainWindow::connectActions()
{
	connect(ui->actionExit, &QAction::triggered, [=]() { onExit(); });
	connect(ui->actionOpen_level, &QAction::triggered, [=]() { onOpenLevel(); });
	connect(ui->actionRestore_layout, &QAction::triggered, [=]() { onRestoreLayout(); });
	connect(ui->actionTypes_Viewer, &QAction::triggered, [=]() { onShowTypesViewer(); });
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

	connect(ui->searchInputField, &QLineEdit::textChanged, [=](const QString &query) { onSearchObjectQueryChanged(query); });

	connect(ui->sceneTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
		if (!selected.indexes().isEmpty()) {
			/// *** STUPID CODE, WILL FIX LATER ***
			const auto* sel = reinterpret_cast<const gamelib::scene::SceneObject*>(selected.indexes().first().constInternalPointer());
			const auto& sceneObjects = editor::EditorInstance::getInstance().getActiveLevel()->getSceneObjects();
			for (int i = 0; i < sceneObjects.size(); ++i)
			{
				if (sceneObjects[i].get() == sel)
				{
					onSelectedSceneObject(i);
					return;
				}
			}
		} else if (!deselected.indexes().isEmpty())
		{
			onDeselectedSceneObject();
		}
	});

	ui->sceneTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
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
	auto sceneModel = qobject_cast<models::SceneObjectsTreeModel *>(ui->sceneTreeView->model());
	if (sceneModel) {
		sceneModel->setLevel(currentLevel);
	}

	auto propertiesModel = qobject_cast<models::SceneObjectPropertiesModel *>(ui->propertiesView->model());
	if (propertiesModel) {
		propertiesModel->setLevel(currentLevel);
	}

	ui->searchInputField->clear();
	ui->searchInputField->setEnabled(true);
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
	ui->sceneTreeView->keyboardSearch(query);
}

void BMEditMainWindow::onSelectedSceneObject(int selectedSceneObjectIdx)
{
	auto sceneObjectPropertiesModel = qobject_cast<models::SceneObjectPropertiesModel *>(ui->propertiesView->model());
	if (!sceneObjectPropertiesModel)
	{
		return;
	}

	auto &entities = editor::EditorInstance::getInstance().getActiveLevel()->getSceneProperties()->header.getEntries().getGeomEntities();
	if (selectedSceneObjectIdx < 0 || selectedSceneObjectIdx >= entities.size())
	{
		return;
	}

	auto &sceneEnt = entities.at(selectedSceneObjectIdx);
	auto type = gamelib::TypeRegistry::getInstance().findTypeByHash(sceneEnt.getTypeId());

	if (!type)
	{
		return;
	}

	ui->sceneObjectName->setText(QString::fromStdString(sceneEnt.getName()));
	ui->sceneObjectTypeCombo->setEnabled(true);
	ui->sceneObjectTypeCombo->setCurrentText(QString::fromStdString(type->getName()));

	sceneObjectPropertiesModel->setGeomIndex(selectedSceneObjectIdx);
}

void BMEditMainWindow::onDeselectedSceneObject()
{
	auto sceneObjectPropertiesModel = qobject_cast<models::SceneObjectPropertiesModel *>(ui->propertiesView->model());
	if (!sceneObjectPropertiesModel)
	{
		return;
	}

	ui->sceneObjectTypeCombo->setEnabled(false);
	ui->sceneObjectName->clear();

	sceneObjectPropertiesModel->resetGeomIndex();
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
		ui->sceneObjectTypeCombo->setModel(new QStringListModel(allAvailableTypes, this));

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