#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include "core/message.h"

class block_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_message(*this));}

	//block messages have only one conent, the ip address to be blocked.
	static message create(const QHostAddress &c) {return message_create("block", new block_message(c));}

	block_message(const QHostAddress &c) : ip(c) {}

	QHostAddress ip;
};

class unblock_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new unblock_message(*this));}

	//unblock messages have only one conent, the ip address to be unblocked.
	static message create(const QHostAddress &c) {return message_create("unblock", new unblock_message(c));}

	unblock_message(const QHostAddress &c) : ip(c) {}

	QHostAddress ip;
};

#endif // BLOCKER_MESSAGES_H
