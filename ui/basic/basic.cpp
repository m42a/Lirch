#include <iostream>
#include <thread>

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"
#include "plugins/display_messages.h"
#include "core/core_messages.h"

using namespace std;

void send_input(plugin_pipe p)
{
	string line;
	while (getline(cin, line))
	{
		//I can't use the regular constructor since that assumes the
		//locale is ASCII
		QString q=QString::fromLocal8Bit(line.c_str());
		p.write(raw_edict_message::create(q,"default"));
	}
	p.write(core_quit_message::create());
}

void run(plugin_pipe p, string name)
{
	thread(send_input, p).detach();
	p.write(registration_message::create(-32000, name, "display"));
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
				cout << channel << ": *" << nick << " " << contents << endl;
			if(s->subtype==display_message_subtype::NOTIFY)
				cout << channel << "‼‽" << contents << endl;
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
