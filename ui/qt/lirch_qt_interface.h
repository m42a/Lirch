#ifndef LIRCH_QT_INTERFACE_H
#define LIRCH_QT_INTERFACE_H

#include <Qt>
#include <QCheckBox>
#include <QCloseEvent>
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
#include "plugins/edict_messages.h"

class LirchClientPipe : public QObject {
    Q_OBJECT
public:
    explicit LirchClientPipe() { client_pipe = nullptr; }
    bool ready() const { return client_pipe != nullptr; }
    void send(const message &m) {
        if (ready()) {
            client_pipe->write(m);
        }
    }
    void display(const display_message &m) {
        emit show(m.channel, m.contents);
    }
    void open(plugin_pipe &pipe, const QString &name) {
        client_name = name;
        client_pipe = &pipe;
        emit run(this);
    }
    void close(const QString &reason = "unknown reason") {
        QString label = client_name;
        client_name.clear();
        client_pipe = nullptr;
        emit shutdown(tr("%1 was closed for %2").arg(label, reason));
    }
private:
    QString client_name;
    plugin_pipe *client_pipe;
signals:
    void run(LirchClientPipe *);
    void shutdown(const QString &);
    void show(const QString &, const QString &);
};

static LirchClientPipe *interconnect;

namespace Ui {
    class LirchQtInterface;
}

class LirchQtInterface : public QMainWindow {
    Q_OBJECT
    friend class LirchClientPipe;
public:
    explicit LirchQtInterface(QWidget *parent = 0);
    virtual ~LirchQtInterface();
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
    // Application settings
    QSettings settings;
    QString nick, default_nick;
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
    void die(const QString &msg = "unknown error");
    void display(const QString &channel, const QString &contents);
    void use(LirchClientPipe *pipe);

protected slots:
    void closeEvent(QCloseEvent *);

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

    void alert_user(const QString &);
    // TODO these need to query the antenna
    void ignore_changed(const QString &, bool);
    void nick_changed(const QString &, bool);
};

#endif // LIRCH_QT_INTERFACE_H
