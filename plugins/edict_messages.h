#ifndef EDICT_MESSAGES_H_
#define EDICT_MESSAGES_H_

#include <QtCore/QString>

#include "core/message.h"

class raw_edict_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new raw_edict_message(*this));}
	static message create(const QString &c, const QString &d) {return message_create("raw_edict", new raw_edict_message(c,d));}

	raw_edict_message(const QString &c, const QString &d) : contents(c), channel(d) {}

	QString contents;
	QString channel;
};

class edict_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new edict_message(*this));}
	static message create(const QString &c, const QString &d) {return message_create("edict", new edict_message(c,d));}

	edict_message(const QString &c, const QString &d) : contents(c), channel(d) {}

	QString contents;
	QString channel;
};

class me_edict_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new me_edict_message(*this));}
	static message create(const QString &c, const QString &d) {return message_create("me_edict", new me_edict_message(c,d));}

	me_edict_message(const QString &c, const QString &d) : contents(c), channel(d) {}

	QString contents;
	QString channel;
};

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

#endif
