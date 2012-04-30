/*
 *
 * The different flavors of notify messages are for human feedback of module actions.
 * Standard Notify are things that will be displayed in the UI.  This should be used for local messages.
 * Sendable Notify are things that you do that other users should see, such as changing your nick, and do get sent over the network.
 * Received notifies should be stored as a received_message of type NOTIFY.  See received_messages.h
 *
 * Notifies to no channel should go to all channels
 */


#ifndef NOTIFY_MESSAGES_H
#define NOTIFY_MESSAGES_H

#include <QString>
#include "core/message.h"

enum class notify_message_subtype
{
	NORMAL, CURRENT_CHANNEL
};

class notify_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new notify_message(*this));}

	//notify messages have only one conent, string to be displayed
	static message create(const QString &chan, const QString &con, const notify_message_subtype & sub = notify_message_subtype::CURRENT_CHANNEL) {return message_create("notify", new notify_message(chan,con,sub));}

	notify_message(const QString &chan, const QString &con, const notify_message_subtype & sub = notify_message_subtype::CURRENT_CHANNEL) : channel(chan), contents(con), type(sub) {}

	QString channel;
	QString contents;
	notify_message_subtype type;
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
