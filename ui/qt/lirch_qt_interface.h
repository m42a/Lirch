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
    // Antenna Slots:
    //   Add to blocklist
    //     Complain if and only if failure to fulfill request
    //   Remove from blocklist
    //   Send broadcast
    //     Complain if and only if issue (sends message to UI)
    //   connect(bool)
    //
    // Antenna Signals:
    //   Broadcast recv from Network
    //     If not in blocklist, package a message and send to core
    //     If in blocklist, ignore

    // Application settings
    QSettings settings;
    QString nick;
    // QString default_save_path;
    bool show_message_timestamps;
    bool show_ignored_messages;
    // How to represent chatArea?
    // Ideas: class extends QTabWidget
    // Registers QTab on channel creation/removal
    // Syncs with View menu
    // QTab contains a single chatArea
    // chatArea model: list of tuples (sender, msg, time)
    // chatArea floats timestamps to the right?
    // chatArea draws (alternating) -----/_____ b/t messages?

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
