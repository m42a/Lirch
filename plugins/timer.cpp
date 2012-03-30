#include <thread>
#include <iostream>

#include "lirch_plugin.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	cerr << "Started" << endl;
	p.write(registration_message::create(0, name, "timer"));
	cerr << "Registered" << endl;
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
				return;
			p.write(timed_message::create(1000));
		}
		else if (m.type=="timer")
		{
			auto t=dynamic_cast<timed_message *>(m.getdata());
			if (!t)
				continue;
			this_thread::sleep_for(chrono::milliseconds(t->msecs));
			p.write(timed_message::create(t->msecs+20));
		}
		else
			p.write(m.decrement_priority());
	}
}
