#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include "core/message.h"

enum block_message_subtype
{
	ADD,REMOVE
};

class block_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_message(*this));}

	//block messages have only one conent, the ip address to be blocked.
	static message create(block_message_subtype sub, const QHostAddress &blockip) {return message_create("block", new block_message(sub, blockip));}

	block_message(block_message_subtype sub, const QHostAddress &blockip) : subtype(sub), ip(blockip) {}

	QHostAddress ip;
	block_message_subtype subtype;
};

#endif // BLOCKER_MESSAGES_H
