#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include "lirch_constants.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QTime>
#include <QObject>
#include <QEvent>
#include <QTimer>

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
public:
    LirchQtInterface(QWidget *parent = 0);
    ~LirchQtInterface();
    bool eventFilter(QObject *object, QEvent *event);

protected:
    void changeEvent(QEvent *e);
    void loadSettings();
    void saveSettings();

private:
    Ui::LirchQtInterface *ui;
    // Antenna, Logger, and Message Pipe references

    // Application settings
    QSettings settings;
    QString nick;
    // QString default_save_path;
    bool show_timestamps;
    bool show_ignored_messages;

private slots:
    void on_actionConnect_triggered(bool);
    void on_actionViewSendButton_toggled(bool);
    void on_actionViewUserList_toggled(bool);
    void on_actionViewIgnored_toggled(bool);
    void on_actionViewTimestamps_toggled(bool);
    void on_actionAbout_triggered();
    void on_msgSendButton_clicked();
};

#endif // LIRCH_QT_INTERFACE_H
