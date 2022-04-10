#include "BMEditMainWindow.h"
#include "ui_BMEditMainWindow.h"

BMEditMainWindow::BMEditMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BMEditMainWindow)
{
    ui->setupUi(this);

	initStatusBar();
	connectActions();
	connectDockWidgetActions();
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