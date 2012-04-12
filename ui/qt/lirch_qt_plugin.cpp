#include "lirch_constants.h"
#include "ui/lirch_client_pipe.h"
#include "plugins/lirch_plugin.h"

void run(plugin_pipe p, std::string name) {
    // Register for the messages that pertain to the GUI
    p.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_DISPLAY));
    extern LirchClientPipe mediator;
    mediator.open(p, QString::fromStdString(name));

    // The mediator will act as a courier to the GUI
    while (mediator.ready()) {
        // Fetch a message from the pipe whenever it arrives
        message m = p.blocking_read();
        // Determine what type of message it is
        if (m.type == LIRCH_MSG_TYPE_SHUTDOWN) {
            mediator.close("core shutdown");
            break;
        } else if (m.type == LIRCH_MSG_TYPE_REG_STAT) {
            // Recieved a registration status message
            auto reg = dynamic_cast<registration_status *>(m.getdata());
            // Or not... in which case, continue reading
            if (!reg) {
                continue;
            }
            if (!reg->status) {
                // Try again to register, if necessary
                if (reg->priority > LIRCH_MSG_PRI_REG_MIN) {
                  // FIXME??? reg->decrement_priority(); instead of -1
                  p.write(registration_message::create(reg->priority - 1, name, reg->type));
                } else {
                  mediator.close("failed to register with core");
                  break;
                }
            }
        } else if (m.type == LIRCH_MSG_TYPE_DISPLAY) {
            auto data = dynamic_cast<display_message *>(m.getdata());
            if (data) {
                // FIXME what about invalid display messages?
                mediator.display(*data);
            }
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
