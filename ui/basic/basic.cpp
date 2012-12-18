#include <iostream>
#include <thread>

#include <QString>

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"
#include "plugins/display_messages.h"
#include "plugins/grinder_messages.h"
#include "plugins/channel_messages.h"
#include "plugins/notify_messages.h"
#include "core/core_messages.h"

using namespace std;

void send_input(plugin_pipe out, plugin_pipe in)
{
	QString channel="default";
	string line;
	while (getline(cin, line))
	{
		//I can't use the regular constructor since that assumes the
		//locale is ASCII
		QString q=QString::fromLocal8Bit(line.c_str());
		if (in.has_message())
		{
			message m=in.read();
			if (auto i=m.try_extract<set_channel_message>())
			{
				channel=i->channel;
			}
		}
		out.write<raw_edict_message>(q,channel);
	}
	out.write<core_quit_message>();
}

struct display_single_channel
{
	static constexpr auto message_id="only";

	//display_single_channel(const QString &chan) : channel(chan) {}

	QString channel;
};

message handle_only(QString text, QString)
{
	if (text.startsWith("/only"))
		return message::create<display_single_channel>(text.remove(0, 6));
	return message::create<empty_message>();
}

void run(plugin_pipe p, string name)
{
	QString channel="";
	bidirectional_message_pipe bmp;
	thread(send_input, p, plugin_pipe(bmp)).detach();
	p.write<registration_message>(-30000, name, "display");
	p.write<registration_message>(-30000, name, "set_channel");
	p.write<registration_message>(-30000, name, "handler_ready");
	p.write<registration_message>(-30000, name, "only");
	p.write<registration_message>(-30000, name, "query_channel");

	while (true)
	{
		message m=p.blocking_read();
		if (m.is<shutdown_message>())
		{
			return;
		}
		else if (auto s=m.try_extract<registration_status>())
		{
			if (!s->status)
			{
				if (s->priority>-32000)
					p.write<registration_message>(s->priority-1, name, s->type);
			}
			else
			{
				if (s->type=="only")
					p.write<register_handler>("/only", handle_only);
			}
		}
		else if (auto s=m.try_extract<display_message>())
		{
			p.write(m.decrement_priority());
			if (channel!="" && channel!=s->channel && s->channel!="")
				continue;

			string sub_channel=s->channel.toLocal8Bit().constData();
			string nick=s->nick.toLocal8Bit().constData();
			string contents=s->contents.toLocal8Bit().constData();

			if(s->subtype==display_message_subtype::NORMAL)
				cout << sub_channel << ": <" << nick << "> " << contents << endl;
			if(s->subtype==display_message_subtype::ME)
				cout << sub_channel << ": * " << nick << " " << contents << endl;
			if(s->subtype==display_message_subtype::NOTIFY)
				cout << sub_channel << QString::fromUtf8(u8": \u203C\u203D").toLocal8Bit().constData() << contents << endl;
			if(s->subtype==display_message_subtype::NOTIFY_CURRENT)
				cout << channel.toLocal8Bit().constData() << QString::fromUtf8(u8": \u203C\u203D").toLocal8Bit().constData() << contents << endl;
		}
		else if (auto i=m.try_extract<set_channel_message>())
		{
			p.write(m.decrement_priority());
			if (i->channel=="")
			{
				if (channel=="")
					p.write<notify_message>("", "On all channels");
				else
					p.write<notify_message>("", "On channel "+channel);
			}
			else
			{
				bmp.core_write(m);
			}
		}
		else if (auto i=m.try_extract<display_single_channel>())
		{
			channel=i->channel;
		}
		else if (m.is<handler_ready>())
		{
			p.write<register_handler>("/only", handle_only);
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
