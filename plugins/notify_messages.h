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
	static message create(const QString &c) {return message_create("local notify", new local_notify_message(c));}

	local_notify_message(const QString &c) : contents(c) {}

	QString contents;
}

class sendable_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new sendable_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &c) {return message_create("sendable notify", new sendable_notify_message(c));}

	sendable_notify_message(const QString &c) : contents(c) {}

	QString contents;
}

class received_notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_notify_message(*this));}

	//local notify messages have only one conent, string to be displayed
	static message create(const QString &c) {return message_create("received notify", new received_notify_message(c));}

	received_message(const QString &c) : contents(c) {}

	QString contents;
}

#endif // NOTIFY_MESSAGES_H
