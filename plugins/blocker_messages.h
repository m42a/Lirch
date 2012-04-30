/*
 * blocker messages should be pretty straightforward.
 * they contain a subtype "add" or "remove" and the ip which this should be performed to.
 */

#ifndef BLOCKER_MESSAGES_H
#define BLOCKER_MESSAGES_H

#include <QHostAddress>
#include <unordered_set>
#include "core/message.h"
#include "QHostAddress_hash.h"

//this nonsense is needed in order to have our blocklist be searchable


enum class block_message_subtype
{
	ADD,REMOVE,QUERY
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

class display_blocks_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_blocks_message(*this));}

	static message create(QString c) {return message_create("display blocks", new display_blocks_message(c));}

	display_blocks_message(QString c) : channel(c) {}

	QString channel;
};

enum class block_name_message_subtype
{
	ADD,REMOVE
};

class block_name_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_name_message(*this));}

	static message create(QString n, QString c, block_name_message_subtype s) {return message_create("block name", new block_name_message(n,c,s));}

	block_name_message(QString n, QString c, block_name_message_subtype s) : name(n), channel(c), type(s) {}

	QString name;
	QString channel;
	block_name_message_subtype type;
};

class block_status_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new block_status_message(*this));}

	static message create(const QHostAddress &blockip, bool stat) {return message_create("block status", new block_status_message(blockip, stat));}

	block_status_message(const QHostAddress &blockip, bool stat) : status(stat), ip(blockip) {}

	bool status;
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
