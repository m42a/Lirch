#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include <QCloseEvent>
#include <QDate>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMap>
#include <QMainWindow>
#include <QMessageBox>
#include <QSet>
#include <QSettings>
#include <QShowEvent>
#include <QString>
#include <QSystemTrayIcon>
#include <QTime>
#include <QTimer>
#include <QUrl>

#include "lirch_constants.h"
#include "ui/lirch_client_pipe.h"
#include "ui/qt/lirch_channel.h"
#include "plugins/blocker_messages.h"
#include "plugins/edict_messages.h"
#include "plugins/display_messages.h"
#include "plugins/logger_messages.h"
#include "plugins/nick_messages.h"

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
public:
    explicit LirchQtInterface(QWidget *parent = nullptr);
    virtual ~LirchQtInterface();
    // TODO filter switch-tab events
    bool eventFilter(QObject *object, QEvent *event);

protected:
    void changeEvent(QEvent *e);

private:
    // Utility functions (for settings)
    void loadSettings();
    void saveSettings();
    // Internals
    Ui::LirchQtInterface *ui;
    LirchClientPipe *client_pipe;
    QSystemTrayIcon *system_tray_icon;
    // Model data (Channels package appropriate data)
    QMap<QString, LirchChannel *> channels;
    QSet<QString> ignored_users;
    // Application settings
    QSettings settings;
    QString default_nick;
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

public slots:
    void die(QString = "unknown error", bool = true);
    void display(QString, QString, QString);
    void userlist(QMap<QString, QSet<QString>>);
    void nick(QString, bool);
    void use(LirchClientPipe *);

protected slots:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

private slots:
    // MENU SLOTS
    // File Menu
    void on_actionNewTransfer_triggered();
    void on_actionNewChannel_triggered();
    void on_actionConnect_triggered(bool);
    void on_actionOpenLog_triggered();
    void on_actionSaveLog_triggered();
    // Edit Menu
    void on_actionEditNick_triggered();
    void on_actionEditIgnored_triggered();
    // View Menu
    void on_actionViewTransfers_triggered();
    void on_actionViewIgnored_toggled(bool);
    void on_actionViewTimestamps_toggled(bool);
    void on_actionViewSendButton_toggled(bool);
    void on_actionViewUserList_toggled(bool);
    // About Menu
    void on_actionWizard_triggered();
    void on_actionAbout_triggered();

    // SEND SLOTS
    void on_msgSendButton_clicked();

    // MISC SLOTS
    void alert_user(QString);
    void request_nick_change(QString, bool);
    void request_block_ignore(QString, bool);
    void request_edict_send(QString, bool);
};

#endif // LIRCH_QT_INTERFACE_H
