#pragma once

#include <GameLib/TEX/TEXEntryType.h>
#include <QShowEvent>
#include <QString>
#include <QDialog>
#include <optional>


namespace Ui {
class ImportTextureDialog;
}

class ImportTextureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportTextureDialog(QWidget *parent = nullptr);
    ~ImportTextureDialog();

	// Setters
	void resetState();
	void setSourcePath(const QString& sourcePath);
	void setTextureName(const QString& textureName);
	void setMIPLevelsCount(uint8_t count);

	// Getters
	[[nodiscard]] const QString& getSourceFilePath() const;
	[[nodiscard]] const QString& getTextureName() const;
	[[nodiscard]] gamelib::tex::TEXEntryType getTargetFormat() const;
	[[nodiscard]] uint8_t getTargetMIPLevels() const;

private:
    Ui::ImportTextureDialog *ui;

	// State
	QString m_sourcePath {};
	QString m_textureName {};
	gamelib::tex::TEXEntryType m_entryType { gamelib::tex::TEXEntryType::ET_BITMAP_32 };
	uint8_t m_mipLevels { 1u };
};
