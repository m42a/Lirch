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

struct register_replacer
{
	static constexpr auto message_id="register_replacer";

	register_replacer(const QString &c, const QRegExp &p, const QString &r, register_replacer_subtype t = register_replacer_subtype::ADD) : command(c), pattern(p), replacement(r),subtype(t) {}

	QString command;
	QRegExp pattern;
	QString replacement;
	register_replacer_subtype subtype;
};

struct replacer_ready
{
	static constexpr auto message_id="replacer_ready";
};

struct register_handler
{
	static constexpr auto message_id="register_handler";

	QString command;
	std::function<message (QString, QString)> handler;
};

struct handler_ready
{
	static constexpr auto message_id="handler_ready";
};

struct display_commands_message
{
	static constexpr auto message_id="display commands";

	QString channel;
	QStringList arguments;
};

struct query_commands_message
{
	static constexpr auto message_id="query commands";
};

struct commands_list_message
{
	static constexpr auto message_id="commands list";

	std::unordered_multimap<QString, std::pair<QRegExp, QString>> text_replacements;
	std::unordered_map<QString, std::function<message (QString, QString)>> handlers;
};

#endif
