#pragma once

#include <QMenu>
#include <QDialog>
#include <QPixmap>
#include <QScopedPointer>
#include <optional>

#include <Types/QTextureREF.h>

namespace Ui {
class ViewTexturesDialog;
}

namespace models
{
class SceneTexturesModel;
class SceneTextureFilterModel;
}

class ImportTextureDialog;


class ViewTexturesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ViewTexturesDialog(QWidget *parent = nullptr);
    ~ViewTexturesDialog();

	void setTexturesSource(models::SceneTexturesModel *model);

signals:
	void textureChanged(uint32_t textureIndex);

protected:
	void showEvent(QShowEvent *event) override;

private slots:
	void onCurrentMipLevelChanged(int mipIndex);
	void onExportTEXFile();
	void onExportCurrentTextureToFile();
	void onReplaceCurrentTexture();
	void onTextureToImportSpecified();

private:
	void setPreview(uint32_t textureIndex, const std::optional<int>& mipLevel = std::nullopt);
	void setPreview(QPixmap &&image);
	void resetPreview();
	void resetAvailableMIPs(uint32_t textureIndex);
	void clearAvailableMIPs();

private:
	[[nodiscard]] std::optional<types::QTextureREF> getActiveTexture() const;
	[[nodiscard]] std::optional<uint8_t> getActiveMIPLevel() const;

private:
    Ui::ViewTexturesDialog *ui;
	QScopedPointer<models::SceneTextureFilterModel> m_filterModel;
	QScopedPointer<QMenu> m_texturePreviewWidgetContextMenu;
	QScopedPointer<ImportTextureDialog> m_importDialog;
};
