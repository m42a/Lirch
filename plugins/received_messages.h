#ifndef RECEIVED_MESSAGES_H
#define RECEIVED_MESSAGES_H

#include <QtNetwork>

#include "core/message.h"

class received_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_message(*this));}

	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(const QString &chan, const QString &nik, const QString &content, const QHostAddress &ip) {return message_create("received", new received_message(chan,nik,content,ip));}

	received_message(const QString &chan, const QString &nik, const QString &conent, const QHostAddress &ip) : channel(chan), nick(nik), contents(conent), ipAddress(ip) {}

	QString channel;
	QString nick;
	QString contents;
	QHostAddress ipAddress;
};

class received_me_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_me_message(*this));}

	//received me messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(const QString &chan, const QString &nik, const QString &content, const QHostAddress &ip) {return message_create("received_me", new received_me_message(chan,nik,content,ip));}

	received_me_message(const QString &chan, const QString &nik, const QString &conent, const QHostAddress &ip) : channel(chan), nick(nik), contents(conent), ipAddress(ip) {}

	QString channel;
	QString nick;
	QString contents;
	QHostAddress ipAddress;
};


#endif // RECEIVED_MESSAGES_H
