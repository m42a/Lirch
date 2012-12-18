#ifndef CHANNEL_MESSAGES_H_
#define CHANNEL_MESSAGES_H_

#include <QStringList>

struct set_channel_message

{
	static constexpr auto message_id="set_channel";

	QString channel;
};

struct leave_channel_message
{
	static constexpr auto message_id="leave_channel";

	QString channel;
};

#endif
