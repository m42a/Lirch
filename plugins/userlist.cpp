#include "lirch_plugin.h"
#include "userlist_messages.h"
#include "user_status.h"

using namespace std;

class blocklist_timer : public message_data
{
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new blocklist_timer(*this));}
	static message create(int m) {return message_create("blocklist_timer", new blocklist_timer(m));}

	blocklist_timer(int m) : msecs(m) {}

	int msecs;
};

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(0, name, "blocklist_request"));
	p.write(registration_message::create(0, name, "blocklist_timer"));
	p.write(registration_message::create(30000, name, "received"));
	p.write(registration_message::create(30000, name, "received_me"));
	unordered_map<QString, user_status> statuses;
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
				if ((0>=s->priority && s->priority>-200) || (30000>=s->priority && s->priority>29000))
					p.write(registration_message::create(s->priority-1, name, s->type));
				else
					return;
			}
			else
			{
				if (s->type=="blocklist_timer")
					p.write(blocklist_timer::create(10000));
			}
		}
		else if (m.type=="blocklist_request")
		{
			p.write(block_message::create(statuses));
		}
		else if (m.type=="blocklist_timer")
		{
			auto s=dynamic_cast<blocklist_timer *>(m.getdata());
			if (!s)
				continue;
			time_t now=time();
			//Remove all nicks that haven't been seen in 2 minutes
			statuses::iterator i=statuses.begin();
			decltype(statuses.begin()) i;
			while ((i=std::find_if(statuses.begin(), statuses.end(), [now](const std::pair<const QString &, const user_status &> &p) {return p.second.lastseen<now-2*60;}))!=statuses.end())
				statuses.erase(i);
			p.write(block_message::create(statuses));
			p.write(blocklist_timer::create(s->msec));
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
