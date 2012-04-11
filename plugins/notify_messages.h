/*
 *
 * The different flavors of notify messages are for human feedback of module actions.
 * Standard Notify are things that will be displayed in the UI.  This should be used for local messages.
 * Sendable Notify are things that you do that other users should see, such as changing your nick, and do get sent over the network.
 * Received notifies should be stored as a received_message of type NOTIFY.  See received_messages.h
 */


#ifndef NOTIFY_MESSAGES_H
#define NOTIFY_MESSAGES_H

#include <QString>
#include "core/message.h"

class notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new notify_message(*this));}

	//notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con) {return message_create("notify", new notify_message(chan,con));}

	notify_message(const QString &chan, const QString &con) : channel(chan), contents(con) {}

	QString channel;
	QString contents;
};

class sendable_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new sendable_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con) {return message_create("sendable_notify", new sendable_notify_message(chan,con));}

	sendable_notify_message(const QString &chan, const QString &con) : channel(chan), contents(con) {}

	QString channel;
	QString contents;
};

#endif // NOTIFY_MESSAGES_H
