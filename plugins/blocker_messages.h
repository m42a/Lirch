/*
 * blocker messages should be pretty straightforward.
 * they contain a subtype "add" or "remove" and the ip which this should be performed to.
 */

#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include <unordered_set>
#include "core/message.h"

//this nonsense is needed in order to have our blocklist be searchable
namespace std
{
	template <>
	struct hash<QHostAddress>
	{
		size_t operator()(const QHostAddress& v) const
		{
			return std::hash<std::string>()(v.toString().toStdString());
		}
	};
}

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

class block_query_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_query_message(*this));}

	static message create() {return message_create("block query", new block_query_message());}

	block_query_message() {}
};

class block_list_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_list_message(*this));}

	static message create(std::unordered_set<QHostAddress> block_list) {return message_create("block list", new block_list_message(block_list));}

	block_list_message(std::unordered_set<QHostAddress> block_list) : blocklist(block_list) {}
	
	std::unordered_set<QHostAddress> blocklist;
};

#endif // BLOCKER_MESSAGES_H
