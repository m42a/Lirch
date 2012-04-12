#include "ui/lirch_client_pipe.h"

// These are rather trivial
LirchClientPipe::LirchClientPipe() :
    client_state(State::BEFORE) { }
// TODO any messages left in the pipe?
LirchClientPipe::~LirchClientPipe() { }

// A client pipe is ready for a duration
bool LirchClientPipe::ready() const {
    return client_state == State::DURING;
}

// The client pipe writes outbound messages
void LirchClientPipe::send(const message &m) {
    if (ready()) {
        client_pipe.write(m);
    }
}

#ifndef NDEBUG
#include <QDebug>
#endif

// The client pipe alerts the client of inbound messages
void LirchClientPipe::display(const display_message &m) {
    QString channel, text, type;
    switch (m.subtype) {
        case display_message_subtype::NORMAL:
            text = tr("<%1> %2").arg(m.nick, m.contents);
            type = tr("/say");
            break;
        case display_message_subtype::ME:
            text = tr("*%1 %2").arg(m.nick, m.contents);
            type = tr("/me");
            break;
        case display_message_subtype::NOTIFY:
            text = tr("!%1 %2").arg(m.nick, m.contents);
            type = tr("/notify");
            break;
        default:
            text = tr("?%1 %2").arg(m.nick, m.contents);
            type = tr("/unknown");
    }
    #ifndef NDEBUG
    QString rep = tr("('%1','%2','%3')").arg(m.channel, m.nick, m.contents);
    qDebug() << tr("Mediator for '%1' forwarded '%2' message: %3").arg(client_name, type, rep);
    #endif
    emit alert(channel, text);
}

// The client pipe signals the UI when it the plugin is run
void LirchClientPipe::open(plugin_pipe &pipe, const QString &name) {
    client_name = name;
    // TODO are we sure this is kosher to copy?
    // FIXME what about any messages in client_pipe?
    client_pipe = pipe;
    client_state = State::DURING;
    emit run(this);
}

// The client pipe signals the UI when the plugin is shutdown
void LirchClientPipe::close(const QString &reason) {
    client_state = State::AFTER;
    emit shutdown(tr("%1 was closed for %2").arg(client_name, reason));
}

