/*
 * blocker messages should be pretty straightforward.
 * they contain a subtype "add" or "remove" and the ip which this should be performed to.
 */

#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include "core/message.h"


enum class block_message_subtype
{
	ADD,REMOVE
};

class block_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_message(*this));}

	static message create(block_message_subtype sub, const QHostAddress &blockip) {return message_create("block", new block_message(sub, blockip));}

	block_message(block_message_subtype sub, const QHostAddress &blockip) : subtype(sub), ip(blockip) {}

	block_message_subtype subtype;
	QHostAddress ip;
};

#endif // BLOCKER_MESSAGES_H
