#include "ui_BMEditMainWindow.h"
#include "BMEditMainWindow.h"
#include "TypeViewerWindow.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>

#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeNotFoundException.h>

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

	initStatusBar();
	connectActions();
	connectDockWidgetActions();
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

	m_operationLabel->setText("Progress: ");
	m_operationCommentLabel->setText("(No active operation)");

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

void BMEditMainWindow::onExit()
{
	close();
}

void BMEditMainWindow::onOpenLevel()
{
	// TODO: Implement open level
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