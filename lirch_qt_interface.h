#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include "lirch_constants.h"
#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
public:
    LirchQtInterface(QWidget *parent = 0);
    ~LirchQtInterface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::LirchQtInterface *ui;

private slots:
    void on_actionAbout_triggered();
    void on_msgSendButton_clicked();
};

#endif // LIRCH_QT_INTERFACE_H
