#include <thread>
#include <iostream>

#include "lirch_plugin.h"

using namespace std;

class timed_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new timed_message(*this));}
	static message create(int m) {return message_create("timer", new timed_message(m));}

	timed_message(int m) : msecs(m) {}

	int msecs;
};

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
