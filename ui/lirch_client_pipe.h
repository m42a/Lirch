#ifndef LIRCH_CLIENT_PIPE_H
#define LIRCH_CLIENT_PIPE_H
#include <thread>

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "core/message_view.h"
#include "plugins/channel_messages.h"
#include "plugins/display_messages.h"
#include "plugins/edict_messages.h"
#include "plugins/nick_messages.h"
#include "plugins/userlist_messages.h"

class LirchClientPipe : public QObject {
    Q_OBJECT
    // BEFORE refers to the period of time prior to open()
    // DURING, prior to close(), and AFTER, thereafter
    // All client pipes have a mode
    enum class State { BEFORE, DURING, AFTER } client_state;
    // All client pipes wrap a plugin pipe for a specific (named) plugin
    QString client_name;
    plugin_pipe client_pipe;
    // In order to join with the core thread, we need this
    std::thread *core_thread;
    // This is just for compliance
    QString notify_prefix;
public:
    explicit LirchClientPipe();
    virtual ~LirchClientPipe();
    void load(std::thread *);
    QString name() const;
    // Used for querying the status on either end
    bool ready() const;
    // Called by the client when sending messages
    void send(message);
    // A client is notified when these are called
    void display(display_message);
    void userlist(userlist_message);
    void nick(changed_nick_message);
    void channel(set_channel_message);
    void channel(leave_channel_message);
    // These each change the state
    void open(plugin_pipe, QString);
    void close(QString = tr("unknown reason"));
public slots:
    // For making sure the core thread joins before shutdown
    void join();
signals:
    // For alerting the UI when to start/stop [show()/close()]
    void run(LirchClientPipe *);
    void shutdown(QString);
    // For alerting the UI of an inbound message
    void display_received(QString, QString, QString);
    void userlist_updated(QMap<QString, QSet<QString>>);
    void nick_changed(QString, bool);
    void set_channel(QString);
    void leave_channel(QString);
};

#endif // LIRCH_CLIENT_PIPE_H
