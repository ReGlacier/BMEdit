#include "ViewTexturesDialog.h"
#include <ImportTextureDialog.h>
#include "ui_ViewTexturesDialog.h"
#include <Models/SceneTexturesModel.h>
#include <Models/SceneTextureFilterModel.h>
#include <Editor/TextureProcessor.h>
#include <GameLib/TEX/TEX.h>
#include <GameLib/Level.h>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>


static constexpr int kDefaultMIP = 0;
static constexpr const char* kPNGFilter = "PNG image (*.png)";


static QString convertTextureTypeToQString(gamelib::tex::TEXEntryType entry)
{
	switch (entry)
	{
		case gamelib::tex::TEXEntryType::ET_BITMAP_I8: return "I8";
		case gamelib::tex::TEXEntryType::ET_BITMAP_EMBM: return "EMBM";
		case gamelib::tex::TEXEntryType::ET_BITMAP_DOT3: return "DOT3";
		case gamelib::tex::TEXEntryType::ET_BITMAP_CUBE: return "CUBE";
		case gamelib::tex::TEXEntryType::ET_BITMAP_DMAP: return "DMAP";
		case gamelib::tex::TEXEntryType::ET_BITMAP_PAL: return "PAL (Neg)";
		case gamelib::tex::TEXEntryType::ET_BITMAP_PAL_OPAC: return "PAL (Opac)";
		case gamelib::tex::TEXEntryType::ET_BITMAP_32: return "RGBA";
		case gamelib::tex::TEXEntryType::ET_BITMAP_U8V8: return "U8V8";
		case gamelib::tex::TEXEntryType::ET_BITMAP_DXT1: return "DXT1";
		case gamelib::tex::TEXEntryType::ET_BITMAP_DXT3: return "DXT3";
	}

	return "Unknown";
}

ViewTexturesDialog::ViewTexturesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ViewTexturesDialog)
{
    ui->setupUi(this);
	ui->texturesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
	ui->texturesTableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->texturesTableView->verticalHeader()->hide();

	m_importDialog.reset(new ImportTextureDialog(this));
	m_filterModel.reset(new models::SceneTextureFilterModel(this));
	ui->texturesTableView->setModel(m_filterModel.get());

	// Connect common signals
	connect(ui->mipLevelCombo, &QComboBox::currentIndexChanged, this, &ViewTexturesDialog::onCurrentMipLevelChanged);
	connect(ui->exportTEXButton, &QPushButton::clicked, this, &ViewTexturesDialog::onExportTEXFile);
	connect(ui->exportTextureButton, &QPushButton::clicked, this, &ViewTexturesDialog::onExportCurrentTextureToFile);
	connect(ui->replaceTextureButton, &QPushButton::clicked, this, &ViewTexturesDialog::onReplaceCurrentTexture);
	connect(ui->searchTextureByNameInput, &QLineEdit::textChanged, [this](const QString& query) {
		m_filterModel->setTextureNameFilter(query);
	});
	connect(m_importDialog.get(), &QDialog::accepted, this, &ViewTexturesDialog::onTextureToImportSpecified);
}

ViewTexturesDialog::~ViewTexturesDialog()
{
    delete ui;
}

void ViewTexturesDialog::setTexturesSource(models::SceneTexturesModel* model)
{
	{
		QSignalBlocker blocker { ui->searchTextureByNameInput };
		ui->searchTextureByNameInput->clear();
		m_filterModel->setTextureNameFilter({});
	}

	m_filterModel->setSourceModel(model);
	ui->mipLevelCombo->clear();
	ui->texturesTableView->resizeColumnsToContents();

	connect(ui->texturesTableView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection &selected, const QItemSelection &deselected) {
		const bool somethingSelected = !selected.indexes().isEmpty();

		if (somethingSelected)
		{
			const QModelIndex mappedSelection = m_filterModel->mapToSource(selected.indexes().first());
			const auto textureRef = mappedSelection.data(models::SceneTexturesModel::Roles::R_TEXTURE_REF).value<types::QTextureREF>();

			if (!textureRef.textureIndex)
				return;

			const auto textureIndex = textureRef.textureIndex;

			// Show available MIP levels
			resetAvailableMIPs(textureIndex);
			setPreview(textureIndex, std::nullopt);
		}
		else if (!deselected.indexes().isEmpty())
		{
			ui->exportTextureButton->setEnabled(false);
			ui->replaceTextureButton->setEnabled(false);
			clearAvailableMIPs();
			resetPreview();
		}
	});
}

void ViewTexturesDialog::showEvent(QShowEvent *event)
{
	ui->mipLevelCombo->clear();
	ui->texturesTableView->selectionModel()->reset();
	ui->searchTextureByNameInput->setText(QString());
	resetPreview();

	QDialog::showEvent(event);
}

void ViewTexturesDialog::onCurrentMipLevelChanged(int mipIndex)
{
	auto textureRefOptional = getActiveTexture();
	if (!textureRefOptional.has_value())
		return;

	auto currentMIPOptional = getActiveMIPLevel();
	if (!currentMIPOptional.has_value())
		return;

	const auto& textureREF = textureRefOptional.value();
	const auto textureIndex = textureREF.textureIndex;

	setPreview(textureIndex, currentMIPOptional.value());
}

void ViewTexturesDialog::onExportTEXFile()
{
	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return;

	QFileDialog saveTEXDialog(this, QString("Save TEX"), QString(), QString("Glacier Texture pack (*.TEX)"));
	saveTEXDialog.setViewMode(QFileDialog::ViewMode::Detail);
	saveTEXDialog.setFileMode(QFileDialog::FileMode::AnyFile);
	saveTEXDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	saveTEXDialog.selectFile(QString("%1.TEX").arg(QString::fromStdString(model->getLevel()->getLevelName())));
	if (!saveTEXDialog.exec())
	{
		return;
	}

	if (saveTEXDialog.selectedFiles().empty())
	{
		return;
	}

	const auto saveAsPath = saveTEXDialog.selectedFiles().first();
	const auto sceneTextures = model->getLevel()->getSceneTextures();

	// Build TEX into buffer
	std::vector<uint8_t> texBuffer;
	gamelib::tex::TEXWriter::write(sceneTextures->header, sceneTextures->entries, texBuffer);

	QFile texFile(saveAsPath);
	if (!texFile.open(QIODeviceBase::OpenModeFlag::WriteOnly | QIODeviceBase::OpenModeFlag::Truncate | QIODeviceBase::OpenModeFlag::Unbuffered))
	{
		QMessageBox::warning(this, "Export TEX", QString("Failed to export TEX file. Unable to open file '%1' to write").arg(saveAsPath));
		return;
	}

	texFile.write(QByteArray::fromRawData(reinterpret_cast<const char*>(texBuffer.data()), static_cast<qsizetype>(texBuffer.size())));
	QMessageBox::information(this, "Export TEX", QString("TEX file '%1' exported successfully!").arg(saveAsPath));
}

void ViewTexturesDialog::onExportCurrentTextureToFile()
{
	auto textureRefOptional = getActiveTexture();
	if (!textureRefOptional.has_value())
		return;

	const auto& textureREF = textureRefOptional.value();

	const auto textureIndex = textureREF.textureIndex;
	const auto& entries = textureREF.ownerModel->getLevel()->getSceneTextures()->entries;
	auto foundIt = std::find_if(entries.begin(), entries.end(), [idx = textureIndex](const gamelib::tex::TEXEntry& entry) -> bool {
		return entry.m_index == idx;
	});

	if (foundIt == entries.end())
		return;

	QString relatedFileName;
	const gamelib::tex::TEXEntry& textureToSave = *foundIt;

	// Predict name
	if (textureToSave.m_fileName.has_value())
	{
		relatedFileName = QString::fromStdString(textureToSave.m_fileName.value());
	}
	else
	{
		relatedFileName = QString("Texture_%1").arg(textureToSave.m_index);
	}

	// Show dialog
	QFileDialog saveTEXDialog(this, QString("Export texture"));
	saveTEXDialog.setNameFilters({ kPNGFilter });
	saveTEXDialog.setViewMode(QFileDialog::ViewMode::Detail);
	saveTEXDialog.setFileMode(QFileDialog::FileMode::AnyFile);
	saveTEXDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	saveTEXDialog.selectFile(relatedFileName);
	if (!saveTEXDialog.exec())
	{
		return;
	}

	if (saveTEXDialog.selectedFiles().empty())
	{
		return;
	}

	const auto savePath = saveTEXDialog.selectedFiles().first();
	const auto saveExt = saveTEXDialog.selectedNameFilter();

	bool exportResult = false;
	if (saveExt == kPNGFilter)
	{
		exportResult = editor::TextureProcessor::exportTEXEntryAsPNG(textureToSave, savePath.toStdString());
	}
	//TODO: Add support for other formats

	if (exportResult)
	{
		QMessageBox::information(this, "Export texture", QString("Texture exported to file %1 successfully!").arg(savePath));
	}
	else
	{
		QMessageBox::warning(this, "Export texture", "Failed to export texture. Unsupported format or internal error");
	}
}

void ViewTexturesDialog::onReplaceCurrentTexture()
{
	auto textureRefOptional = getActiveTexture();
	if (!textureRefOptional.has_value())
		return;

	const auto& textureREF = textureRefOptional.value();

	const auto textureIndex = textureREF.textureIndex;
	auto& entries = textureREF.ownerModel->getLevel()->getSceneTextures()->entries;
	auto foundIt = std::find_if(entries.begin(), entries.end(), [idx = textureIndex](const gamelib::tex::TEXEntry& entry) -> bool {
		return entry.m_index == idx;
	});

	// Cleanup
	m_importDialog->resetState();

	// Update state
	if (foundIt->m_fileName.has_value())
	{
		m_importDialog->setTextureName(QString::fromStdString(foundIt->m_fileName.value()));
	}
	m_importDialog->setMIPLevelsCount(foundIt->m_mipLevels.size());

	// And open dialog again
	m_importDialog->setModal(true);
	m_importDialog->open();
}

void ViewTexturesDialog::onTextureToImportSpecified()
{
	if (ui->texturesTableView->selectionModel()->selectedIndexes().isEmpty())
		return;

	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return;

	// Extract data & select texture to replace
	const auto& sourceFile = m_importDialog->getSourceFilePath();
	const auto& textureName = m_importDialog->getTextureName();
	const auto targetFormat = m_importDialog->getTargetFormat();
	const auto mipLevelsNr = m_importDialog->getTargetMIPLevels();

	// Check that format supported
	if (targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_PAL_OPAC ||  // Maybe will support in future
	    targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_EMBM     ||  // DirectX 9 stuff, no support from us
	    targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_DOT3     ||  // DirectX 9 stuff, no support from us
	    targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_CUBE     ||  // DirectX 9 stuff, no support from us
	    targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_DMAP)        // DirectX 9 stuff, no support from us
	{
		QMessageBox::warning(this, "Import texture", "Required target format not supported yet");
		return;
	}

	auto textureRefOptional = getActiveTexture();
	if (!textureRefOptional.has_value())
		return;

	const auto& textureREF = textureRefOptional.value();

	const auto textureIndex = textureREF.textureIndex;
	auto& entries = const_cast<gamelib::Level*>(textureREF.ownerModel->getLevel())->getSceneTextures()->entries;
	auto foundIt = std::find_if(entries.begin(), entries.end(), [idx = textureIndex](const gamelib::tex::TEXEntry& entry) -> bool {
		return entry.m_index == idx;
	});

	if (!editor::TextureProcessor::importTextureToEntry(*foundIt, sourceFile, textureName, targetFormat, mipLevelsNr))
	{
		QMessageBox::warning(this, "Import texture", "Failed to import texture! Invalid or unsupported format");
		return;
	}

	// And it's done! Update our preview!
	resetPreview();
	setPreview(textureIndex, std::nullopt);
	resetAvailableMIPs(textureIndex);

	// Notify everybody about changes
	emit textureChanged(textureREF.textureIndex);
}

void ViewTexturesDialog::setPreview(uint32_t textureIndex, const std::optional<int>& mipLevel)
{
	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return;

	auto& entries = const_cast<gamelib::Level*>(model->getLevel())->getSceneTextures()->entries;
	auto foundIt = std::find_if(entries.begin(), entries.end(), [textureIndex](const gamelib::tex::TEXEntry& entry) -> bool {
		return entry.m_index == textureIndex;
	});

	const auto& textureEntry = *foundIt;

	// Select MIP level
	int requiredMipLevel = mipLevel.has_value() ? mipLevel.value() : kDefaultMIP;

	if (requiredMipLevel < 0 || requiredMipLevel >= textureEntry.m_mipLevels.size())
	{
		if (textureEntry.m_mipLevels.empty())
		{
			qDebug() << "No mip levels in texture #" << textureIndex;
			return;
		}

		qDebug() << "Bad mip level. Use #0";
		requiredMipLevel = 0;
	}

	uint16_t width = 0;
	uint16_t height = 0;
	std::unique_ptr<std::uint8_t[]> decompressedMemBlk = editor::TextureProcessor::decompressRGBA(textureEntry, width, height, requiredMipLevel);

	// Save to QPixmap
	if (decompressedMemBlk)
	{
		QImage image(decompressedMemBlk.get(), width, height, QImage::Format::Format_RGBA8888);
		QPixmap pixmap = QPixmap::fromImage(std::move(image));
		setPreview(std::move(pixmap));

		ui->formatLabel->setText(convertTextureTypeToQString(textureEntry.m_type1));

		ui->exportTextureButton->setEnabled(true);
		ui->replaceTextureButton->setEnabled(true);
	}
	else
	{
		qDebug() << "Unsupported format";
		resetPreview();
	}
}

void ViewTexturesDialog::setPreview(QPixmap &&image)
{
	resetPreview();
	ui->textureViewWidget->setPixmap(image);
}

void ViewTexturesDialog::resetPreview()
{
	ui->textureViewWidget->setPixmap(QPixmap());
	ui->formatLabel->setText("Unknown");
	ui->exportTextureButton->setEnabled(false);
	ui->replaceTextureButton->setEnabled(false);
}

void ViewTexturesDialog::resetAvailableMIPs(uint32_t textureIndex)
{
	QSignalBlocker blocker { ui->mipLevelCombo };
	ui->mipLevelCombo->clear();

	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return;

	auto& entries = const_cast<gamelib::Level*>(model->getLevel())->getSceneTextures()->entries;
	auto foundIt = std::find_if(entries.begin(), entries.end(), [idx = textureIndex](const gamelib::tex::TEXEntry& entry) -> bool {
		return entry.m_index == idx;
	});

	const auto& textureEntry = *foundIt;

	for (uint32_t mip = 0; mip < textureEntry.m_numOfMipMaps; ++mip)
	{
		ui->mipLevelCombo->addItem(QString("MIP #%1").arg(mip), mip);
	}

	ui->mipLevelCombo->setEnabled(textureEntry.m_numOfMipMaps > 1);
}

void ViewTexturesDialog::clearAvailableMIPs()
{
	QSignalBlocker blocker { ui->mipLevelCombo };
	ui->mipLevelCombo->clear();
}

std::optional<types::QTextureREF> ViewTexturesDialog::getActiveTexture() const
{
	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return std::nullopt;

	if (ui->texturesTableView->selectionModel()->selectedIndexes().empty())
		return std::nullopt;

	QModelIndex mappedSelection = m_filterModel->mapToSource(ui->texturesTableView->selectionModel()->selectedIndexes().first());

	auto textureREF = mappedSelection.data(models::SceneTexturesModel::Roles::R_TEXTURE_REF).value<types::QTextureREF>();
	if (!textureREF.textureIndex)
		return std::nullopt;

	return textureREF;
}

std::optional<uint8_t> ViewTexturesDialog::getActiveMIPLevel() const
{
	auto model = dynamic_cast<models::SceneTexturesModel*>(m_filterModel->sourceModel());
	if (!model)
		return std::nullopt;

	if (ui->texturesTableView->selectionModel()->selectedIndexes().empty())
		return std::nullopt;

	if (!ui->mipLevelCombo->isEnabled())
		return kDefaultMIP;

	return static_cast<uint8_t>(ui->mipLevelCombo->itemData(ui->mipLevelCombo->currentIndex(), Qt::UserRole).value<uint32_t>());
}