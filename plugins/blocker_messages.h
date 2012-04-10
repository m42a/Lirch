/*
 * blocker messages should be pretty straightforward.
 * they contain a subtype "add" or "remove" and the ip which this should be performed to.
 */

#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include "core/message.h"

namespace block_message_subtype
{
	enum Enum
	{
		ADD,REMOVE
	};
}

class block_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_message(*this));}

	static message create(block_message_subtype::Enum sub, const QHostAddress &blockip) {return message_create("block", new block_message(sub, blockip));}

	block_message(block_message_subtype::Enum sub, const QHostAddress &blockip) : subtype(sub), ip(blockip) {}

	QHostAddress ip;
	block_message_subtype::Enum subtype;
};

#endif // BLOCKER_MESSAGES_H
