#include "lirch_constants.h"
#include "ui/lirch_client_pipe.h"
#include "plugins/lirch_plugin.h"

void run(plugin_pipe p, std::string name) {
    // Register for the messages that pertain to the GUI
    p.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_DISPLAY));
    p.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_USERLIST));
    extern LirchClientPipe mediator;
    mediator.open(p, QString::fromStdString(name));

    // The mediator will act as a courier to the GUI
    while (mediator.ready()) {
        // Fetch a message from the pipe whenever it arrives
        message m = p.blocking_read();
        // Determine what type of message it is
        if (m.type == LIRCH_MSG_TYPE_SHUTDOWN) {
            mediator.close(QObject::tr("core shutdown"));
            break;
        } else if (m.type == LIRCH_MSG_TYPE_REG_STAT) {
            // Recieved a registration status message
            auto reg = dynamic_cast<registration_status *>(m.getdata());
            if (reg && !reg->status) {
                int registration_priority = reg->priority;
                // Retry over 9000 times until we succeed
                if (registration_priority < LIRCH_MSG_PRI_REG_MIN) {
                  mediator.close(QObject::tr("failed to register with core"));
                  break;
		} else {
                  // Try again to register, if necessary
                  p.write(registration_message::create(--registration_priority, name, reg->type));
                }
            }
        } else if (m.type == LIRCH_MSG_TYPE_DISPLAY) {
            auto data = dynamic_cast<display_message *>(m.getdata());
            if (data) {
                mediator.display(*data);
            }
        } else if (m.type == LIRCH_MSG_TYPE_USERLIST) {
            auto data = dynamic_cast<userlist_message *>(m.getdata());
            if (data) {
                mediator.userlist(*data);
            }
            p.write(m.decrement_priority());
        } else {
            // By default, echo the message with decremented priority
            p.write(m.decrement_priority());
        }
    }

    // We only get here through anomalous behavior (though it is good to have a catch-all)
    if (mediator.ready()) {
        mediator.close();
    }
}
