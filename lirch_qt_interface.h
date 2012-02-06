#ifndef CLIENT_H
#define CLIENT_H

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

#endif // CLIENT_H
