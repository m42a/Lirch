#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include <QEvent>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QTime>
#include <QTimer>

#include "lirch_constants.h"
#include <string>
#include "core/message_view.h"

class LirchClientPipe : public QObject {
    Q_OBJECT
    friend class LirchQtInterface;
public:
    explicit LirchClientPipe();
    ~LirchClientPipe();
private:
    plugin_pipe * hole;
public slots:
    void start();
signals:
    void stop(QString);
};

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
public:
    explicit LirchQtInterface(QWidget *parent = 0);
    ~LirchQtInterface();
    bool eventFilter(QObject *object, QEvent *event);

protected:
    void changeEvent(QEvent *e);

private:
    void loadSettings();
    void saveSettings();
    Ui::LirchQtInterface *ui;
    LirchClientPipe *client_pipe;
    // Application settings
    QSettings settings;
    QString nick;
    // QString default_save_path;
    bool show_message_timestamps;
    bool show_ignored_messages;

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

    // How to represent chatArea?
    // Ideas: class extends QTabWidget
    // Registers QTab on channel creation/removal
    // Syncs with View menu
    // QTab contains a single chatArea
    // chatArea model: list of tuples (sender, msg, time)
    // chatArea floats timestamps to the right?
    // chatArea draws (alternating) -----/_____ b/t messages?

private slots:
    void on_actionOpenLog_triggered();
    void on_actionSaveLog_triggered();
    void on_actionWizard_triggered();
    void on_actionViewTransfers_triggered();
    void on_actionViewDefault_triggered();
    void on_actionNewTransfer_triggered();
    void on_actionNewChannel_triggered();
    void on_actionConnect_triggered(bool);
    void on_actionViewSendButton_toggled(bool);
    void on_actionViewUserList_toggled(bool);
    void on_actionViewIgnored_toggled(bool);
    void on_actionViewTimestamps_toggled(bool);
    void on_actionAbout_triggered();
    void on_msgSendButton_clicked();
    void fatal_error(QString);

signals:
    void tabChanged(int);
};

#endif // LIRCH_QT_INTERFACE_H
