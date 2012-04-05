#include <iostream>
#include <thread>

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"

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
}

void run(plugin_pipe p, string name)
{
	thread t(send_input, p);
	p.write(registration_message::create(-32000, name, "display"));
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			t.join();
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
			cout << s->channel.toLocal8Bit().constData() << ": <" << s->nick.toLocal8Bit().constData() << "> " << s->contents.toLocal8Bit().constData() << endl;
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
