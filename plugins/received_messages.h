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


class received_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_message(*this));}

	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(received_message_subtype sub, const QString &chan, const QString &nik, const QString &content, const QHostAddress &ip) {return message_create("received", new received_message(sub,chan,nik,content,ip));}

	received_message(received_message_subtype sub, const QString &chan, const QString &nik, const QString &conent, const QHostAddress &ip) : subtype(sub), channel(chan), nick(nik), contents(conent), ipAddress(ip) {}

	received_message_subtype subtype;
	QString channel;
	QString nick;
	QString contents;
	QHostAddress ipAddress;
};

class received_status_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_status_message(*this));}

	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(received_status_message_subtype sub, const QString &chan, const QString &nik, const QHostAddress &ip) {return message_create("received_status", new received_status_message(sub,chan,nik,ip));}

	received_status_message(received_status_message_subtype sub, const QString &chan, const QString &nik, const QHostAddress &ip) : subtype(sub), channel(chan), nick(nik), ipAddress(ip) {}

	received_status_message_subtype subtype;
	QString channel;
	QString nick;
	QHostAddress ipAddress;
};

#endif // RECEIVED_MESSAGES_H
