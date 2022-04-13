#ifndef TYPEVIEWERWINDOW_H
#define TYPEVIEWERWINDOW_H

#include <QDialog>


namespace Ui {
class TypeViewerWindow;
}

class TypeViewerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TypeViewerWindow(QWidget *parent = nullptr);
    ~TypeViewerWindow();

private:
    Ui::TypeViewerWindow *ui;
};

#endif // TYPEVIEWERWINDOW_H
