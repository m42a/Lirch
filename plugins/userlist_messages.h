#ifndef USERLIST_MESSAGES_H_
#define USERLIST_MESSAGES_H_

#include <unordered_map>
#include <QCore/QString>

#include "core/message.h"
#include "user_status.h"

class blocklist_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new blocklist_message(*this));}
	static message create(const unordered_map<QString, user_status> &m) {return message_create("blocklist", new blocklist_message(m));}

	blocklist_message(const unordered_map<QString, user_status> &m) : statuses(m) {}

	unordered_map<QString, user_status> statuses;
};

class blocklist_request : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(nullptr);}
	static message create() {return message_create("blocklist_request", nullptr);}
};

#endif
