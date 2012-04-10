#ifndef RECEIVED_MESSAGES_H
#define RECEIVED_MESSAGES_H

#include <QtNetwork>

#include "core/message.h"

namespace received_message_subtype
{
	enum Enum
	{
		NORMAL,ME,NOTIFY
	};
}

class received_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_message(*this));}

	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(received_message_subtype::Enum sub, const QString &chan, const QString &nik, const QString &content, const QHostAddress &ip) {return message_create("received", new received_message(sub,chan,nik,content,ip));}

	received_message(received_message_subtype::Enum sub, const QString &chan, const QString &nik, const QString &conent, const QHostAddress &ip) : subtype(sub), channel(chan), nick(nik), contents(conent), ipAddress(ip) {}

	QString channel;
	QString nick;
	QString contents;
	QHostAddress ipAddress;
	received_message_subtype::Enum subtype;
};

#endif // RECEIVED_MESSAGES_H
