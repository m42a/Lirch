/*
 *
 * The different flavors of notify messages are for human feedback of module actions.
 * Local Notify are things that other users on the system don't have any need to know, such as blocking someone, and should not be sent over the network.
 * Sendable Notify are things that you do that other users should see, such as changing your nick, and do get sent over the network.
 * Received Notify are other users' notifies received from the network.
 */


#ifndef NOTIFY_MESSAGES_H
#define NOTIFY_MESSAGES_H

#include <QString>
#include "core/message.h"

class local_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new local_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con) {return message_create("local_notify", new local_notify_message(chan,con));}

	local_notify_message(const QString &chan, const QString &con) : channel(chan), contents(con) {}

	QString contents;
	QString channel;
};

class sendable_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new sendable_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con) {return message_create("sendable_notify", new sendable_notify_message(chan,con));}

	sendable_notify_message(const QString &chan, const QString &con) : channel(chan), contents(con) {}

	QString contents;
	QString channel;
};

class received_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con) {return message_create("received_notify", new received_notify_message(chan,con));}

	received_notify_message(const QString &chan, const QString &con) : channel(chan), contents(con) {}

	QString contents;
	QString channel;
};

#endif // NOTIFY_MESSAGES_H
