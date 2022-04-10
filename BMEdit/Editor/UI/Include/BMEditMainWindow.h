#ifndef BMEDITMAINWINDOW_H
#define BMEDITMAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>


namespace Ui {
class BMEditMainWindow;
}

class BMEditMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BMEditMainWindow(QWidget *parent = nullptr);
    ~BMEditMainWindow();

private:
	void initStatusBar();
	void connectActions();
	void connectDockWidgetActions();

public slots:
	void onExit();
	void onOpenLevel();
	void onRestoreLayout();

private:
    Ui::BMEditMainWindow *ui;
    // Custom
    QLabel* m_operationLabel;
    QLabel* m_operationCommentLabel;
    QProgressBar* m_operationProgress;
};

#endif // BMEDITMAINWINDOW_H
