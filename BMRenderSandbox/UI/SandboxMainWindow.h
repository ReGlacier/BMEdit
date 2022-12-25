#ifndef SANDBOXMAINWINDOW_H
#define SANDBOXMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class SandboxMainWindow;
}

class SandboxMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SandboxMainWindow(QWidget *parent = nullptr);
    ~SandboxMainWindow();

public slots:
	void onSelectLevelToOpenRequested();
	void onExitRequested();
	void onLevelLoaded();
	void onLevelLoadFailed(const QString &errorMessage);

private:
    Ui::SandboxMainWindow *ui;
};

#endif // SANDBOXMAINWINDOW_H
