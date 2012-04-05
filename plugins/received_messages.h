#ifndef RECEIVED_MESSAGES_H
#define RECEIVED_MESSAGES_H

#include "core/message.h"

class received_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_message(*this));}

	//received messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(const QString &c, const QString &d, const QString &e) {return message_create("received", new received_message(c,d,e));}

	received_message(const QString &c, const QString &d, const QString &e) : channel(c), nick(d), conents(e) {}

	QString channel;
	QString nick;
	QString contents;
};

class received_me_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new received_me_message(*this));}

	//received me messages have three contents, the channel to display to, the nick of the sender, and the contents of the broadcast.
	static message create(const QString &c, const QString &d, const QString &e) {return message_create("received_me", new received_me_message(c,d,e));}

	received_me_message(const QString &c, const QString &d, const QString &e) : channel(c), nick(d), conents(e) {}

	QString channel;
	QString nick;
	QString contents;
};


#endif // RECEIVED_MESSAGES_H
