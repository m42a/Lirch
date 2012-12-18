#ifndef USERLIST_MESSAGES_H_
#define USERLIST_MESSAGES_H_

#include <unordered_map>
#include <QtCore/QString>

#include "core/message.h"
#include "user_status.h"

struct userlist_message
{
	static constexpr auto message_id="userlist";

	QString currentNick;
	std::unordered_map<QString, user_status> statuses;
};

struct userlist_request
{
	static constexpr auto message_id="userlist_request";
};

struct who_is_here_message
{
	static constexpr auto message_id="who is here";

	QString channel;
};

struct here_message
{
	static constexpr auto message_id="here";

	QString channel;
};

struct who_is_message
{
	static constexpr auto message_id="who_is";

	QString channel;
	QString nick;
};

#endif
