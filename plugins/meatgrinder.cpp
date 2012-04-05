#include "lirch_plugin.h"
#include "edict_messages.h"
#include "core/core_messages.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	//int pri=32000;
	p.write(registration_message::create(32000, name, "raw_edict"));
	while (true)
	{
		message m=p.blocking_read();
		if (m.type=="shutdown")
		{
			//We don't have any cleanup to do, so just return
			return;
		}
		else if (m.type=="registration_status")
		{
			auto s=dynamic_cast<registration_status *>(m.getdata());
			if (!s)
				continue;
			//Retry 2000 times until we succeed
			if (!s->status && s->priority>30000)
				p.write(registration_message::create(s->priority-1, name, s->type));
		}
		else if (m.type=="raw_edict")
		{
			auto e=dynamic_cast<raw_edict_message *>(m.getdata());
			if (!e)
				continue;
			auto str=e->contents;
			if (str.isEmpty())
				//We don't propagate empty messages (on principle)
				continue;
			if (str[0]!='/')
				p.write(edict_message::create(str));
			else
			{
				if (str.startsWith("/me "))
					p.write(me_edict_message::create(str.section(' ',1)));
				if (str=="/quit" || str.startsWith("/quit "))
					p.write(core_quit_message::create());
			}
		}
		else
		{
			p.write(m.decrement_priority());
		}
	}
}
