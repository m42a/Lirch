#include <thread>
#include <iostream>

#include "lirch_plugin.h"
#include "timer.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(0, name, "timer"));
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
				if (s->priority>-200)
					p.write(registration_message::create(s->priority-1, name, "timer"));
				else
					return;
			}
		}
		else if (m.type=="timer")
		{
			auto t=dynamic_cast<timed_message *>(m.getdata());
			if (!t)
				continue;
			thread th([&p](int msecs, message m)
			{
				this_thread::sleep_for(chrono::milliseconds(msecs));
				p.write(m);
			}, t->msecs, t->m);
			th.detach();
		}
		else
			p.write(m.decrement_priority());
	}
}
