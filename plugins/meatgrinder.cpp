#include <unordered_map>

#include <QString>

#include "lirch_plugin.h"
#include "QString_hash.h"
#include "edict_messages.h"
#include "notify_messages.h"
#include "grinder_messages.h"
#include "core/core_messages.h"
#include "blocker_messages.h"
#include "notify_messages.h"
#include "channel_messages.h"
#include "parser.h"


using namespace std;

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
	if (!text.startsWith("/channel"))
		return empty_message::create();
	return set_channel::create(text.remove(0, 9));
}

message handle_commands(QString text, QString channel)
{
	if (!text.startsWith("/commands"))
		return empty_message::create();
	QStringList parsed = parse(text);
	parsed.removeFirst();
	return display_commands_message::create(channel, parsed);
}

message handle_erase_command(QString text, QString)
{
	if(!text.startsWith("/macro_erase "))
		return empty_message::create();
	text.remove(0, 13);
	while(text[0] == ' ')
		text.remove(0, 1);
	QStringList parsed = parse(text);
	QString command = parsed.at(0);
	command.push_front("/");
	if(parsed.size() < 1)
		return empty_message::create();
	while(parsed.size() < 3)
		parsed += "";
	return register_replacer::create(command, QRegExp(parsed.at(1), Qt::CaseSensitive, QRegExp::RegExp2), parsed.at(2), register_replacer_subtype::REMOVE);
}

message handle_channel_leave(QString text, QString)
{
	if (!text.startsWith("/leave"))
		return empty_message::create();
	return leave_channel::create(text.remove(0, 7));
}

message handle_user_command(QString text, QString)
{
	if (!text.startsWith("/macro "))
		return empty_message::create();
	text.remove(0, 7);
	while(text[0] == ' ')
		text.remove(0, 1);
	QStringList parsed = parse(text);
	QString command = parsed.at(0);
	command.push_front("/");
	if(parsed.size() < 3)
		return empty_message::create();
	return register_replacer::create(command, QRegExp(parsed.at(1), Qt::CaseSensitive, QRegExp::RegExp2), parsed.at(2));
}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(-30000, name, "raw_edict"));
	p.write(registration_message::create(-30000, name, "register_replacer"));
	p.write(registration_message::create(-30000, name, "register_handler"));
	p.write(registration_message::create(-30000, name, "display commands"));
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
					p.write(register_handler::create("/commands", handle_commands));
					p.write(register_handler::create("/leave", handle_channel_leave));
					p.write(register_handler::create("/macro", handle_user_command));
					p.write(register_handler::create("/macro_erase", handle_erase_command));
				}
				else if (s->type=="register_replacer")
				{
					p.write(replacer_ready::create());
					p.write(register_replacer::create("/slap", QRegExp("/slap\\b( ?) *(.*)", Qt::CaseSensitive, QRegExp::RegExp2), "/me slaps\\1\\2 with an optimistic biologist"));
				}
			}
		}
		else if (m.type=="register_replacer")
		{
			auto i=dynamic_cast<register_replacer *>(m.getdata());
			if (!i)
				continue;
			if(i->subtype == register_replacer_subtype::ADD)
				text_replacements.insert({i->command, {i->pattern, i->replacement}});
			else if(i->subtype == register_replacer_subtype::REMOVE)
			{
				if(i->pattern.pattern() == "")
					text_replacements.erase(text_replacements.find(i->command));
				else if(text_replacements.count(i->command) > 0)
					for(unordered_multimap<QString, pair<QRegExp, QString> >::iterator iter = text_replacements.find(i->command); iter!=text_replacements.end(); iter++)
					{
						if(i->command == iter->first && i->pattern == iter->second.first && (i->replacement == "" || i->replacement == iter->second.second))
						{
							text_replacements.erase(iter);
							break;
						}
					}
			}
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
		else if (m.type == "display commands")
		{
			auto internals=dynamic_cast<display_commands_message *>(m.getdata());
			if (!internals)
				continue;
			QString output;
			if(internals->arguments.size() == 0)
			{
				output = "commands:";
				for(unordered_map<QString, function<message (QString, QString)>>::iterator i = handlers.begin(); i != handlers.end(); i++)
				{	
					if(i->first != "")
					{
						output += "\n";
						output += i->first;
					}
				}
				for(unordered_multimap<QString, pair<QRegExp, QString>>::iterator i = text_replacements.begin(); i != text_replacements.end(); i++)
				{	
					if(i->first != "")
					{
						output += "\n";
						output += i->first;
					}
				}
			}
			for(int argument = 0; argument < internals->arguments.size(); argument++)
			{
				if(internals->arguments[argument] == "macros")
				{
					output = "macros:";
					for(unordered_multimap<QString, pair<QRegExp, QString>>::iterator i = text_replacements.begin(); i != text_replacements.end(); i++)
						if(i->first != "")
							output.append("\n").append(i->first).append(" \"").append(i->second.first.pattern()).append("\" \"").append(i->second.second).append("\"");
				}
			}
			p.write(notify_message::create(internals->channel, output));
		}
		else if (m.type == "query commands")
		{
			
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
