#ifndef LIRCH_CLIENT_PIPE_H
#define LIRCH_CLIENT_PIPE_H
#include <thread>

#include <QObject>
#include <QString>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "core/message_view.h"
#include "plugins/display_messages.h"
#include "plugins/edict_messages.h"

class LirchClientPipe : public QObject {
    Q_OBJECT
    // BEFORE refers to the period of time prior to open()
    // DURING, prior to close(), and AFTER, thereafter
    enum class State { BEFORE, DURING, AFTER };
public:
    explicit LirchClientPipe();
    virtual ~LirchClientPipe();
    void load(std::thread *);
    // Used for querying the status on either end
    bool ready() const;
    // Called by the client when sending messages
    void send(message);
    // A client is notified when these are called
    void display(display_message);
    // These each change the state
    void open(plugin_pipe, QString);
    void close(QString = "unknown reason");
private:
    // All client pipes have a mode, see above
    State client_state;
    // All client pipes wrap a plugin pipe for a specific (named) plugin
    QString client_name;
    plugin_pipe client_pipe;
    // FIXME somehow refer to the core_thread
    std::thread *core_thread;
public slots:
    void join();
signals:
    // For alerting the UI when to start/stop [show()/close()]
    void run(LirchClientPipe *);
    void shutdown(QString);
    // For alerting the UI of an inbound message
    void alert(QString, QString);
};

#endif // LIRCH_CLIENT_PIPE_H
