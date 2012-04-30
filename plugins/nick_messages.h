/*
 *
 * nick messages ask for the nick to be changed.
 * changed nick messages are for the userlist to send out so everyone can tell when it does.
 */

#ifndef NICK_MESSAGES_H
#define NICK_MESSAGES_H

#include "core/message.h"

class nick_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new nick_message(*this));}

	static message create(const QString nick, bool changeDefault=false) {return message_create("nick", new nick_message(nick, changeDefault));}

	nick_message(const QString nik, bool changeDef=false) : nick(nik), changeDefault(changeDef) {}

	QString nick;
	bool changeDefault;
};

class changed_nick_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new changed_nick_message(*this));}

	static message create(const QString oldNick,const QString newNick,bool wasDefault=false) {return message_create("changed_nick", new changed_nick_message(oldNick,newNick,wasDefault));}

	changed_nick_message(const QString oldNik, const QString newNik,bool isDefault=false) : oldNick(oldNik), newNick(newNik), wasDefault(isDefault){}

	QString oldNick;
	QString newNick;
	bool wasDefault;
};

#endif // NICK_MESSAGES_H
