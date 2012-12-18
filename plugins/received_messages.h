/*
 * received messages are anything received from the network and put into the program.
 * they have four types, Normal, Me, Notify, and Here
 * Normal is a standard broadcast and should be displayed normally
 * Me are for /me broadcasts
 * Notify are for incoming notifications
 * Here messages are for the periodic broadcasts the antenna sends when the user is inactive.  They don't need channel or contents and should not be displayed.
 * Who Here messages are sent by other users trying to know who's in the channel.
 */


#ifndef RECEIVED_MESSAGES_H
#define RECEIVED_MESSAGES_H

#include <QtNetwork>

#include "core/message.h"

enum class received_message_subtype
{
	NORMAL,ME,NOTIFY
};

enum class received_status_message_subtype
{
	LEFT,HERE,WHOHERE,NICK,JOIN
};

struct received_message
{
	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static constexpr auto message_id="received";

	received_message_subtype subtype;
	QString channel;
	QString nick;
	QString contents;
	QHostAddress ipAddress;
};

struct received_status_message
{
	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static constexpr auto message_id="received_status";

	received_status_message_subtype subtype;
	QString channel;
	QString nick;
	QHostAddress ipAddress;
};

#endif // RECEIVED_MESSAGES_H
