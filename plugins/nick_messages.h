/*
 *
 * nick messages ask for the nick to be changed.
 * changed nick messages are for the userlist to send out so everyone can tell when it does.
 */

#ifndef NICK_MESSAGES_H
#define NICK_MESSAGES_H

#include "core/message.h"

struct nick_message
{

	static constexpr auto message_id="nick";

	QString nick;
	bool changeDefault;
};

struct changed_nick_message
{
	static constexpr auto message_id="changed_nick";

	QString oldNick;
	QString newNick;
	bool wasDefault;
};

#endif // NICK_MESSAGES_H
