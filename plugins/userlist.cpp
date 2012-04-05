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
	p.write(blocklist_timer::create(10000));
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
				if (s->priority>-200)
					p.write(registration_message::create(s->priority-1, name, s->type));
				else
					return;
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
			p.write(block_message::create(statuses));
			p.write(blocklist_timer::create(s->msec));
		}
	}
}
