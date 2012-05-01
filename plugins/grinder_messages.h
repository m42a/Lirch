#ifndef GRINDER_MESSAGES_H_
#define GRINDER_MESSAGES_H_

#include <functional>

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QStringList>
#include <unordered_map>
#include <set>

#include "core/message.h"
#include "QString_hash.h"

enum class register_replacer_subtype
{
	ADD,REMOVE
};
class register_replacer : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new register_replacer(*this));}
	static message create(const QString &c, const QRegExp &p, const QString &r, register_replacer_subtype t = register_replacer_subtype::ADD) {return message_create("register_replacer", new register_replacer(c,p,r,t));}

	register_replacer(const QString &c, const QRegExp &p, const QString &r, register_replacer_subtype t = register_replacer_subtype::ADD) : command(c), pattern(p), replacement(r),subtype(t) {}

	QString command;
	QRegExp pattern;
	QString replacement;
	register_replacer_subtype subtype;
};

class replacer_ready : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(nullptr);}
	static message create() {return message_create("replacer_ready", nullptr);}
};

class register_handler : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new register_handler(*this));}
	static message create(const QString &c, const std::function<message (QString, QString)> &h) {return message_create("register_handler", new register_handler(c,h));}

	register_handler(const QString &c, const std::function<message (QString, QString)> &h) : command(c), handler(h) {}

	QString command;
	std::function<message (QString, QString)> handler;
};

class handler_ready : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(nullptr);}
	static message create() {return message_create("handler_ready", nullptr);}
};

class display_commands_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_commands_message(*this));}
	static message create(QString Channel, QStringList Arguments) {return message_create("display commands", new display_commands_message(Channel, Arguments));}

	display_commands_message(QString Channel, QStringList Arguments) : channel(Channel), arguments(Arguments) {}

	QString channel;
	QStringList arguments;
};

class query_commands_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new query_commands_message(*this));}
	static message create() {return message_create("query commands", nullptr);}

	query_commands_message() {};
	
};

class commands_list_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new commands_list_message(*this));}
	static message create(std::unordered_multimap<QString, std::pair<QRegExp, QString> > & tr, std::unordered_map<QString, std::function<message (QString, QString)>> & h) {return message_create("query commands", new commands_list_message(tr, h));}
	
	commands_list_message(std::unordered_multimap<QString, std::pair<QRegExp, QString> > & tr, std::unordered_map<QString, std::function<message (QString, QString)>> & h) : text_replacements(tr), handlers(h) {};
	std::unordered_multimap<QString, std::pair<QRegExp, QString>> text_replacements;
	std::unordered_map<QString, std::function<message (QString, QString)>> handlers;
};

#endif
