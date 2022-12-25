#include <SandboxMainWindow.h>
#include <ui_SandboxMainWindow.h>
#include <QMessageBox>
#include <QFileDialog>


SandboxMainWindow::SandboxMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SandboxMainWindow)
{
    ui->setupUi(this);

	connect(ui->primitivePreview, &SceneViewWidget::levelLoaded, this, &SandboxMainWindow::onLevelLoaded);
	connect(ui->primitivePreview, &SceneViewWidget::levelLoadFailed, this, &SandboxMainWindow::onLevelLoadFailed);

	connect(ui->primitiveSelectorCombo, &QComboBox::currentIndexChanged, [&](int index) {
		auto selectedIndex = static_cast<std::uint32_t>(ui->primitiveSelectorCombo->currentData().value<size_t>());
		ui->primitivePreview->setActivePrimitiveIndex(selectedIndex);
		ui->primitivePreview->moveCameraPositionOutOfPrimitive();
	});

	connect(ui->cameraPositionX, &QDoubleSpinBox::valueChanged, [&](double x) {
		auto pos = ui->primitivePreview->getCameraPosition();
		pos.x = static_cast<float>(x);
		ui->primitivePreview->setCameraPosition(pos);
	});

	connect(ui->cameraPositionY, &QDoubleSpinBox::valueChanged, [&](double y) {
		auto pos = ui->primitivePreview->getCameraPosition();
		pos.y = static_cast<float>(y);
		ui->primitivePreview->setCameraPosition(pos);
	});

	connect(ui->cameraPositionZ, &QDoubleSpinBox::valueChanged, [&](double z) {
		auto pos = ui->primitivePreview->getCameraPosition();
		pos.z = static_cast<float>(z);
		ui->primitivePreview->setCameraPosition(pos);
	});
}

SandboxMainWindow::~SandboxMainWindow()
{
    delete ui;
}

void SandboxMainWindow::onSelectLevelToOpenRequested()
{
	QFileDialog openLevelDialog(this, QString("Open Level"), QString(), QString("Hitman Blood Money level package (*.ZIP)"));
	openLevelDialog.setViewMode(QFileDialog::Detail);
	openLevelDialog.setFileMode(QFileDialog::ExistingFile);
	if (!openLevelDialog.exec())
	{
		return;
	}

	//TODO: Show preloader
	ui->primitivePreview->requestLoadLevel(openLevelDialog.selectedFiles().first());
}

void SandboxMainWindow::onExitRequested()
{
	close();
}

void SandboxMainWindow::onLevelLoaded()
{
	// TODO: Hide preloader

	// Enumerate all levels
	ui->primitiveSelectorCombo->setEnabled(true);
	ui->primitiveSelectorCombo->clear();
	auto geometry = ui->primitivePreview->getGameLevel()->getLevelGeometry();

	for (size_t chunkId = 1; chunkId < geometry->chunks.size(); ++chunkId)
	{
		auto &chunk = geometry->chunks.at(chunkId);

		if (chunk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER)
		{
			continue;
		}

		ui->primitiveSelectorCombo->addItem(QString("%1").arg(chunkId), QVariant(chunkId));
	}

	//ui->primitivePreview->setActivePrimitiveIndex(342); // Main
	//ui->primitivePreview->setActivePrimitiveIndex(331); // LOD
}

void SandboxMainWindow::onLevelLoadFailed(const QString &errorMessage)
{
	// TODO: Hide preloader
	QMessageBox::critical(this, "Failed to load level", errorMessage);
}