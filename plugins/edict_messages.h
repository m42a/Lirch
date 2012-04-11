/*
 * raw_edict and edict were kept separate because modules are unlikely to take in both types.
 * they contain the destination channel and the contents of the message.
 * in the case of edict, it has a subtype of Normal or Me, in order for them to be used in the appropriate manner
 */


#ifndef EDICT_MESSAGES_H_
#define EDICT_MESSAGES_H_

#include <QString>

#include "core/message.h"


enum class edict_message_subtype
{
	NORMAL,ME
};

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
	static message create(edict_message_subtype sub, const QString &chan, const QString &cont) {return message_create("edict", new edict_message(sub,chan,cont));}

	edict_message(edict_message_subtype sub, const QString &chan, const QString &cont) : subtype(sub), channel(chan), contents(cont) {}

	edict_message_subtype subtype;
	QString channel;
	QString contents;
};

#endif
