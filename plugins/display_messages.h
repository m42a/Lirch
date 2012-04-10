/*
 * Display messages contain the destination channel, the contents of the message, and the sender's nick.
 * They come in 3 subtypes, Normal, Me, and Notify, which UIs should display differently in accordance with our display standard.
 * Display messages of Notify subtype will not use the nick data and it can be initialized with an empty string for simplicity.
 */


#ifndef DISPLAY_MESSAGES_H
#define DISPLAY_MESSAGES_H

#include <QString>
#include "core/message.h"

namespace display_message_subtype
{
	enum Enum
	{
		NORMAL,ME,NOTIFY
	};
}


class display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_message(*this));}
	static message create(const display_message_subtype::Enum sub, const QString &chan, const QString &cont, const QString &nik) {return message_create("display", new display_message(sub, chan, cont, nik));}

	display_message(const display_message_subtype::Enum sub, const QString &chan, const QString &cont, const QString &nik) : subtype(sub), channel(chan), contents(cont), nick(nik)  {}

	QString channel;
	QString contents;
	QString nick;
	display_message_subtype::Enum subtype;
};

#endif // DISPLAY_MESSAGES_H
