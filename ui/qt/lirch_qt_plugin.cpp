#include "lirch_constants.h"
#include "ui/lirch_client_pipe.h"
#include "plugins/lirch_plugin.h"

void run(plugin_pipe p, std::string name) {
    // Register for the messages that pertain to the GUI
    p.write<registration_message>(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_DISPLAY);
    p.write<registration_message>(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_USERLIST);
    p.write<registration_message>(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_CHANGED_NICK);
    p.write<registration_message>(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_SET_CHANNEL);
    p.write<registration_message>(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_LEAVE_CHANNEL);
    extern LirchClientPipe mediator;
    mediator.open(p, QString::fromStdString(name));

    // The mediator will act as a courier to the GUI
    while (mediator.ready()) {
        // Fetch a message from the pipe whenever it arrives
        message m = p.blocking_read();
        // Determine what type of message it is
        if (m.is<shutdown_message>()) {
            mediator.close(QObject::tr("core shutdown"));
            break;
        } else if (auto reg = m.try_extract<registration_status>()) {
            // Recieved a registration status message
            if (!reg->status) {
                int registration_priority = reg->priority;
                // Retry over 9000 times until we succeed
                if (registration_priority < LIRCH_MSG_PRI_REG_MIN) {
                  mediator.close(QObject::tr("failed to register with core"));
                  break;
		} else {
                  // Try again to register, if necessary
                  p.write<registration_message>(--registration_priority, name, reg->type);
                }
            }
        } else if (auto data = m.try_extract<display_message>()) {
            mediator.display(*data);
            p.write(m.decrement_priority());
        } else if (auto data = m.try_extract<userlist_message>()) {
            mediator.userlist(*data);
            p.write(m.decrement_priority());
        } else if (auto data = m.try_extract<changed_nick_message>()) {
            mediator.nick(*data);
            p.write(m.decrement_priority());
	} else if (auto data = m.try_extract<set_channel_message>()) {
            mediator.channel(*data);
            p.write(m.decrement_priority());
	} else if (auto data = m.try_extract<leave_channel_message>()) {
            mediator.channel(*data);
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
