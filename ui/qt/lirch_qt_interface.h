#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include <Qt>
#include <QCheckBox>
#include <QEvent>
#include <QFileDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QTime>
#include <QTimer>

#include "lirch_constants.h"
#include "core/message_view.h"

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
public:
    explicit LirchQtInterface(plugin_pipe &client_pipe, QWidget *parent = 0);
    ~LirchQtInterface();
    bool eventFilter(QObject *object, QEvent *event);

protected:
    void changeEvent(QEvent *e);

private:
    void loadSettings();
    void saveSettings();
    Ui::LirchQtInterface *ui;
    plugin_pipe *client_pipe;
    // Application settings
    QSettings settings;
    QString nick, default_nick;
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
    void on_actionEditIgnored_triggered();
    void on_actionEditNick_triggered();
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

    void alert_user(QString);
    void close_prompt();
    // TODO these need to query the antenna
    void ignore_changed(QString, bool);
    void nick_changed(QString, bool);    

public slots:
    void display_message(QString, QString);
    void fatal_error(QString msg = "unknown error");
};

#endif // LIRCH_QT_INTERFACE_H
