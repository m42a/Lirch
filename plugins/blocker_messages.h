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

enum class block_message_subtype
{
	ADD,REMOVE,QUERY
};

struct block_message
{
	static constexpr auto message_id="block";

	block_message_subtype subtype;
	QHostAddress ip;
};

struct display_blocks_message
{
	static constexpr auto message_id="display blocks";

	QString channel;
};

enum class block_name_message_subtype
{
	ADD,REMOVE
};

struct block_name_message
{
	static constexpr auto message_id="block name";

	QString name;
	QString channel;
	block_name_message_subtype type;
};

struct block_status_message
{
	static constexpr auto message_id="block status";

	QHostAddress ip;
	bool status;
};

struct block_query_message
{
	static constexpr auto message_id="block query";
};

struct block_list_message
{
	static constexpr auto message_id="block list";

	std::unordered_set<QHostAddress> blocklist;
};

#endif // BLOCKER_MESSAGES_H
