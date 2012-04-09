#ifndef DISPLAY_MESSAGES_H
#define DISPLAY_MESSAGES_H

#include <QString>
#include "core/message.h"

class display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_message(*this));}
	static message create(const QString &ch, const QString &c, const QString &nik) {return message_create("display", new display_message(ch, c, nik));}

	display_message(const QString &ch, const QString &c, const QString &nik) : contents(c), channel(ch), nick(nik)  {}

	QString channel;
	QString contents;
	QString nick;
};

class me_display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new me_display_message(*this));}
	static message create(const QString &ch, const QString &c, const QString &nik) {return message_create("me_display", new me_display_message(ch, c, nik));}

	me_display_message(const QString &ch, const QString &c, const QString &nik) : contents(c), channel(ch), nick(nik)  {}

	QString channel;
	QString contents;
	QString nick;
};

class notify_display_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new notify_display_message(*this));}
	static message create(const QString &ch, const QString &c) {return message_create("notify_display", new notify_display_message(ch, c));}

	notify_display_message(const QString &ch, const QString &c) : contents(c), channel(ch)  {}

	QString channel;
	QString contents;
};

#endif // DISPLAY_MESSAGES_H
