#ifndef GRINDER_MESSAGES_H_
#define GRINDER_MESSAGES_H_

#include <functional>

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include "core/message.h"

class register_replacer : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new register_replacer(*this));}
	static message create(const QString &c, const QRegExp &p, const QString &r) {return message_create("register_replacer", new register_replacer(c,p,r));}

	register_replacer(const QString &c, const QRegExp &p, const QString &r) : command(c), pattern(p), replacement(r) {}

	QString command;
	QRegExp pattern;
	QString replacement;
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

#endif
