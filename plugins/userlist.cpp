#include <thread>

#include "lirch_plugin.h"
#include "userlist_messages.h"
#include "user_status.h"
#include "received_messages.h"

using namespace std;

class userlist_timer : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new userlist_timer(*this));}
	static message create(int m) {return message_create("userlist_timer", new userlist_timer(m));}

	userlist_timer(int m) : msecs(m) {}

	int msecs;
};

void thread_notify(plugin_pipe pin, plugin_pipe pout)
{
	while (true)
	{
		message m=pin.blocking_read();
		if (m.type=="userlist_timer")
		{
			auto s=dynamic_cast<userlist_timer *>(m.getdata());
			if (!s)
				continue;
			this_thread::sleep_for(chrono::milliseconds(s->msecs));
			pout.write(userlist_timer::create(s->msecs));
		}
		else
			return;
	}
}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(0, name, "userlist_request"));
	p.write(registration_message::create(0, name, "userlist_timer"));
	p.write(registration_message::create(30000, name, "received"));
	p.write(registration_message::create(30000, name, "received_me"));
	unordered_map<QString, user_status> statuses;
	bidirectional_message_pipe bmp;
	thread t(thread_notify, plugin_pipe(bmp), p);
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			bmp.core_write(shutdown_message::create());
			t.join();
			return;
		}
		else if (m.type=="registration_status")
		{
			auto s=dynamic_cast<registration_status *>(m.getdata());
			if (!s)
				continue;
			if (!s->status)
			{
				if ((0>=s->priority && s->priority>-200) || (30000>=s->priority && s->priority>29000))
					p.write(registration_message::create(s->priority-1, name, s->type));
				else
					return;
			}
			else
			{
				if (s->type=="userlist_timer")
					p.write(userlist_timer::create(10000));
			}
		}
		else if (m.type=="userlist_request")
		{
			p.write(userlist_message::create(statuses));
		}
		else if (m.type=="userlist_timer")
		{
			auto s=dynamic_cast<userlist_timer *>(m.getdata());
			if (!s)
				continue;
			bmp.core_write(m);
			time_t now=time(NULL);
			//Remove all nicks that haven't been seen in 2 minutes
			decltype(statuses.begin()) i;
			while ((i=std::find_if(statuses.begin(), statuses.end(), [now](const std::pair<const QString &, const user_status &> &p) {return p.second.lastseen<now-2*60;}))!=statuses.end())
				statuses.erase(i);
			p.write(userlist_message::create(statuses));
		}
		else if (m.type=="received")
		{
			auto s=dynamic_cast<received_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());
			statuses[s->nick].nick=s->nick;
			statuses[s->nick].channels.insert(s->channel);
			statuses[s->nick].ip=s->ipAddress;
			statuses[s->nick].lastseen=time(NULL);
		}
		else if (m.type=="received_me")
		{
			auto s=dynamic_cast<received_me_message *>(m.getdata());
			if (!s)
				continue;
			p.write(m.decrement_priority());
			statuses[s->nick].nick=s->nick;
			statuses[s->nick].channels.insert(s->channel);
			statuses[s->nick].ip=s->ipAddress;
			statuses[s->nick].lastseen=time(NULL);
		}
		else
			p.write(m.decrement_priority());
	}
}
