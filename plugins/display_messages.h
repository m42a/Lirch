/*
 * Display messages contain the destination channel, the contents of the message, and the sender's nick.
 * They come in 3 subtypes, Normal, Me, and Notify, which UIs should display differently in accordance with our display standard.
 * Display messages of Notify subtype will not use the nick data and it can be initialized with an empty string for simplicity.
 */

#ifndef DISPLAY_MESSAGES_H
#define DISPLAY_MESSAGES_H

#include <QString>
#include "core/message.h"

// Oliver, no more of these. *slap*
enum class display_message_subtype
{
	NORMAL,ME,NOTIFY,NOTIFY_CURRENT
};

// These are created for the massuese
struct display_message
{
	static constexpr auto message_id="display";

	display_message_subtype subtype;
	QString channel;
	QString nick;
	QString contents;
};

#endif // DISPLAY_MESSAGES_H
