#ifndef DISPLAY_MESSAGES_H
#define DISPLAY_MESSAGES_H

#include <QString>
#include "core/message.h"

enum display_message_subtype
{
	NORMAL,ME,NOTIFY
};

class display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_message(*this));}
	static message create(const display_message_subtype sub, const QString &chan, const QString &cont, const QString &nik) {return message_create("display", new display_message(sub, chan, cont, nik));}

	display_message(const display_message_subtype sub, const QString &chan, const QString &cont, const QString &nik) : subtype(sub), channel(chan), contents(cont), nick(nik)  {}

	QString channel;
	QString contents;
	QString nick;
	display_message_subtype subtype;
};

#endif // DISPLAY_MESSAGES_H
