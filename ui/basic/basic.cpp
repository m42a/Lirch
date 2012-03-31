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
		p.write(raw_edict_message::create(q));
	}
}

void run(plugin_pipe p, string name)
{
	thread t(send_input, p);
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			return;
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
