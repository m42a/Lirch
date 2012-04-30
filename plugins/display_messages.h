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
class display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_message(*this));}
	static message create(const display_message_subtype sub, const QString &chan, const QString &nik, const QString &cont) {return message_create("display", new display_message(sub, chan, nik, cont));}

	display_message(const display_message_subtype sub, const QString &chan, const QString &nik, const QString &cont) : subtype(sub), channel(chan), contents(cont), nick(nik)  {}

	display_message_subtype subtype;
	QString channel;
	QString contents;
	QString nick;
};

#endif // DISPLAY_MESSAGES_H
