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

struct notify_message
{
	//notify messages have only one conent, string to be displayed
	static constexpr auto message_id="notify";

	notify_message(const QString &chan, const QString &con, const notify_message_subtype & sub = notify_message_subtype::CURRENT_CHANNEL) : channel(chan), contents(con), type(sub) {}

	QString channel;
	QString contents;
	notify_message_subtype type;
};

struct sendable_notify_message
{
	//local notify messages have only one conent, string to be displayed
	static constexpr auto message_id="sendable_notify";

	QString channel;
	QString contents;
};

#endif // NOTIFY_MESSAGES_H
