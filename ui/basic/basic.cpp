#include <iostream>
#include <thread>

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"
#include "plugins/display_messages.h"
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

void run(plugin_pipe p, string name)
{
	bidirectional_message_pipe bmp;
	thread(send_input, p, plugin_pipe(bmp)).detach();
	p.write(registration_message::create(-30000, name, "display"));
	p.write(registration_message::create(-30000, name, "set_channel"));
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			return;
		}
		else if (m.type=="registration_status")
		{
			//Handle this
		}
		else if (m.type=="display")
		{
			auto s=dynamic_cast<display_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());

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
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
