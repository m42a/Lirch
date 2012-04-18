#include <unordered_map>

#include <QString>

#include "lirch_plugin.h"
#include "edict_messages.h"
#include "notify_messages.h"
#include "grinder_messages.h"
#include "core/core_messages.h"
#include "blocker_messages.h"
#include "notify_messages.h"
#include "channel_messages.h"

using namespace std;

namespace std
{
	template <>
	struct hash<QString>
	{
		size_t operator()(const QString& v) const
		{
			return std::hash<std::string>()(v.toStdString());
		}
	};
}

QString prefix(const QString &s)
{
	if (s.isEmpty() || s[0]!='/')
		return "";
	return s.left(s.indexOf(' '));
}

QString run_replacers(const unordered_multimap<QString, pair<QRegExp, QString>> &replacements, QString message)
{
	auto range=replacements.equal_range(prefix(message));
	for (auto &i=range.first; i!=range.second; ++i)
		message.replace(i->second.first, i->second.second);
	return message;
}

message handle_me(QString text, QString channel)
{
	if (text=="/me")
		return empty_message::create();
	//Remove the leading "/me "
	return edict_message::create(edict_message_subtype::ME, channel, text.remove(0,4));
}

message handle_quit(QString, QString)
{
	return core_quit_message::create();
}

message handle_normal(QString text, QString channel)
{
	if (text.startsWith("/say "))
		text.remove(0,5);
	return edict_message::create(edict_message_subtype::NORMAL, channel, text);
}

message handle_channel_change(QString text, QString)
{
	if (!text.startsWith("/channel "))
		return empty_message::create();
	return set_channel::create(text.remove(0, 9));
}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(-30000, name, "raw_edict"));
	p.write(registration_message::create(-30000, name, "register_replacer"));
	p.write(registration_message::create(-30000, name, "register_handler"));
	unordered_multimap<QString, pair<QRegExp, QString>> text_replacements;
	unordered_map<QString, function<message (QString, QString)>> handlers;
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			//We don't have any cleanup to do, so just return
			return;
		}
		else if (m.type=="registration_status")
		{
			auto s=dynamic_cast<registration_status *>(m.getdata());
			if (!s)
				continue;
			//Retry 2000 times until we succeed
			if (!s->status)
			{
				if (s->priority>-32000)
					p.write(registration_message::create(s->priority-1, name, s->type));
			}
			else
			{
				if (s->type=="register_handler")
				{
					p.write(handler_ready::create());
					p.write(register_handler::create("", handle_normal));
					p.write(register_handler::create("/say", handle_normal));
					p.write(register_handler::create("/me", handle_me));
					p.write(register_handler::create("/q", handle_quit));
					p.write(register_handler::create("/quit", handle_quit));
					p.write(register_handler::create("/channel", handle_channel_change));
				}
				else if (s->type=="register_replacer")
				{
					p.write(replacer_ready::create());
					p.write(register_replacer::create("/slap", QRegExp("/slap (.*)"), "/me slaps \\1 with an optimistic biologist"));
				}
			}
		}
		else if (m.type=="register_replacer")
		{
			auto i=dynamic_cast<register_replacer *>(m.getdata());
			if (!i)
				continue;
			text_replacements.insert({i->command, {i->pattern, i->replacement}});
		}
		else if (m.type=="register_handler")
		{
			auto i=dynamic_cast<register_handler *>(m.getdata());
			if (!i)
				continue;
			handlers.insert({i->command, i->handler});
		}
		else if (m.type=="raw_edict")
		{
			auto e=dynamic_cast<raw_edict_message *>(m.getdata());
			if (!e)
				continue;
			//Ensure that characters with multiple representations
			//are normalized so we can compare strings directly by
			//character.
			auto str=e->contents.normalized(QString::NormalizationForm_C);
			if (str.isEmpty())
				//We don't propagate empty messages (on principle)
				continue;
			auto pre=prefix(str);
			auto mod=run_replacers(text_replacements, str);
			if (prefix(mod)!=pre)
			{
				//Recurse if the type of the message has been changed
				p.write(raw_edict_message::create(mod, e->channel));
				continue;
			}
			if (handlers.count(pre)==0)
				//Nothing can handle this type of message, so complain
				//We should be using tr here since this is a
				//message to be displayed, but I'm not sure
				//which tr to use.
				p.write(notify_message::create(e->channel, QString("Unknown message type \"%1\"").arg(pre)));
			else
				p.write(handlers[pre](str, e->channel));
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
