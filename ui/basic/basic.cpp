#include <iostream>
#include <thread>

#include <QString>

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"
#include "plugins/display_messages.h"
#include "plugins/grinder_messages.h"
#include "plugins/channel_messages.h"
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
			if (m.type=="set_channel")
			{
				auto i=dynamic_cast<set_channel *>(m.getdata());
				if (i)
					channel=i->channel;
			}
		}
		out.write(raw_edict_message::create(q,channel));
	}
	out.write(core_quit_message::create());
}

class display_single_channel : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new display_single_channel(*this));}

	//notify messages have only one conent, string to be displayed
	static message create(const QString &chan) {return message_create("only", new display_single_channel(chan));}

	display_single_channel(const QString &chan) : channel(chan) {}

	QString channel;
};

message handle_only(QString text, QString)
{
	if (text.startsWith("/only"))
		return display_single_channel::create(text.remove(0, 6));
	return empty_message::create();
}

void run(plugin_pipe p, string name)
{
	QString channel="";
	bidirectional_message_pipe bmp;
	thread(send_input, p, plugin_pipe(bmp)).detach();
	p.write(registration_message::create(-30000, name, "display"));
	p.write(registration_message::create(-30000, name, "set_channel"));
	p.write(registration_message::create(-30000, name, "handler_ready"));
	p.write(registration_message::create(-30000, name, "only"));
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			return;
		}
		else if (m.type=="registration_status")
		{
			auto s=dynamic_cast<registration_status *>(m.getdata());
			if (!s)
				continue;
			if (!s->status)
			{
				if (s->priority>-32000)
					p.write(registration_message::create(s->priority-1, name, s->type));
			}
			else
			{
				if (s->type=="only")
					p.write(register_handler::create("/only", handle_only));
			}
		}
		else if (m.type=="display")
		{
			auto s=dynamic_cast<display_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());
			if (channel!="" && channel!=s->channel)
				continue;

			string channel=s->channel.toLocal8Bit().constData();
			string nick=s->nick.toLocal8Bit().constData();
			string contents=s->contents.toLocal8Bit().constData();

			if(s->subtype==display_message_subtype::NORMAL)
				cout << channel << ": <" << nick << "> " << contents << endl;
			if(s->subtype==display_message_subtype::ME)
				cout << channel << ": * " << nick << " " << contents << endl;
			if(s->subtype==display_message_subtype::NOTIFY)
				cout << channel << ": ‼‽ " << contents << endl;
		}
		else if (m.type=="set_channel")
		{
			auto i=dynamic_cast<set_channel *>(m.getdata());
			if (!i)
				continue;
			bmp.core_write(m);
			p.write(m.decrement_priority());
		}
		else if (m.type=="only")
		{
			auto i=dynamic_cast<display_single_channel *>(m.getdata());
			if (!i)
				continue;
			channel=i->channel;
		}
		else if (m.type=="handler_ready")
		{
			p.write(register_handler::create("/only", handle_only));
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
