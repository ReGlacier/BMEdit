#include "ImportTextureDialog.h"
#include "ui_ImportTextureDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>


ImportTextureDialog::ImportTextureDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ImportTextureDialog)
{
    ui->setupUi(this);

	resetState();

	connect(ui->selectSourceTextureButton, &QPushButton::clicked, [this]() {
		QFileDialog openTextureDialog(this, QString("Select Texture..."));
		openTextureDialog.setViewMode(QFileDialog::ViewMode::Detail);
		openTextureDialog.setFileMode(QFileDialog::FileMode::AnyFile);
		openTextureDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
		openTextureDialog.setNameFilters({ "PNG Texture (*.png)", "JPG Texture (*.jpg)" });

		if (!openTextureDialog.exec())
		{
			return;
		}

		if (openTextureDialog.selectedFiles().empty())
		{
			return;
		}

		QString textureToImport = openTextureDialog.selectedFiles().first();

		// Try update preview
		QImage image { textureToImport };
		if (image.isNull())
		{
			// Bruh
			QMessageBox::warning(this, "Import texture", QString("Failed to import texture '%1'. Unsupported format!").arg(textureToImport));
			return;
		}

	  ui->previewBox->setTitle(QString("Preview (%1;%2):").arg(image.width()).arg(image.height()));
	  ui->previewWidget->setPixmap(QPixmap::fromImage(std::move(image)));

		// And set up
		setSourcePath(openTextureDialog.selectedFiles().first());
	});

	connect(ui->importButton, &QPushButton::clicked, [this]() {
		if (!m_sourcePath.isEmpty())
		{
			accept();
		}
		else
		{
			QMessageBox::warning(this, "Import texture", "You must specify source texture path before import!");
		}
	});

	connect(ui->destinationFormatCombo, &QComboBox::currentIndexChanged, [this](int index) {
		m_entryType = static_cast<gamelib::tex::TEXEntryType>(ui->destinationFormatCombo->itemData(index, Qt::UserRole).value<uint32_t>());
	});

	connect(ui->mipLevels, &QSpinBox::valueChanged, [this](int value) {
		m_mipLevels = static_cast<uint8_t>(value);
	});

	connect(ui->textureName, &QLineEdit::textChanged, [this](const QString& newName) {
		m_textureName = newName;
	});
}

ImportTextureDialog::~ImportTextureDialog()
{
    delete ui;
}

void ImportTextureDialog::resetState()
{
	// Clear data
	m_sourcePath = {};
	m_textureName = {};
	m_entryType = gamelib::tex::TEXEntryType::ET_BITMAP_32;
	m_mipLevels = 1u;

	// Clear path
	{
	    QSignalBlocker blocker { ui->sourcePathEdit };
		ui->sourcePathEdit->clear();
	}

	// Clear name
	{
	    QSignalBlocker blocker { ui->textureName };
		ui->textureName->clear();
	}

	// Fill known and supported formats
	{
		QSignalBlocker blocker { ui->destinationFormatCombo };
		ui->destinationFormatCombo->clear();
		ui->destinationFormatCombo->addItem("RGBA", static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_32));
		ui->destinationFormatCombo->addItem("I8",   static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_I8));
		ui->destinationFormatCombo->addItem("PALN", static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_PAL));
		ui->destinationFormatCombo->addItem("U8V8", static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_U8V8));
		ui->destinationFormatCombo->addItem("DXT1", static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_DXT1));
		ui->destinationFormatCombo->addItem("DXT3", static_cast<uint32_t>(gamelib::tex::TEXEntryType::ET_BITMAP_DXT3));
	}

	// Reset mip level
	{
		QSignalBlocker blocker { ui->mipLevels };
		ui->mipLevels->setValue(1);
	}

	// Reset preview
	ui->previewWidget->setPixmap(QPixmap());
	ui->previewBox->setTitle("Preview:");
}

void ImportTextureDialog::setSourcePath(const QString& sourcePath)
{
	m_sourcePath = sourcePath;
	ui->sourcePathEdit->setText(m_sourcePath);
}

void ImportTextureDialog::setTextureName(const QString& textureName)
{
	m_textureName = textureName;
	ui->textureName->setText(textureName);
}

void ImportTextureDialog::setMIPLevelsCount(uint8_t count)
{
	if (count > 0)
	{
		m_mipLevels = count;
		ui->mipLevels->setValue(count);
	}
}

const QString& ImportTextureDialog::getSourceFilePath() const
{
	return m_sourcePath;
}

const QString &ImportTextureDialog::getTextureName() const
{
	return m_textureName;
}

gamelib::tex::TEXEntryType ImportTextureDialog::getTargetFormat() const
{
	return m_entryType;
}

uint8_t ImportTextureDialog::getTargetMIPLevels() const
{
	return m_mipLevels;
}
