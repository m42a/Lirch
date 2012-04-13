#include <thread>
#include <ctime>
#include <iostream>

#include <QtNetwork>

#include "lirch_plugin.h"
#include "core/core_messages.h"
#include "received_messages.h"
#include "userlist_messages.h"
#include "user_status.h"

using namespace std;

bool test_userlist(plugin_pipe p)
{
	message m=p.blocking_read();
	if (m.type!="userlist")
		return false;
	auto u=dynamic_cast<userlist_message *>(m.getdata());
	if (!u)
		return false;
	if (!u->statuses.empty())
		return false;
	p.write(received_message::create(received_message_subtype::NORMAL, "POTUS", "James K Polk", "Invade Texas!", QHostAddress("3.4.18.45")));
	this_thread::sleep_for(chrono::seconds(11));
	m=p.read();
	if (m.type!="userlist")
		return false;
	u=dynamic_cast<userlist_message *>(m.getdata());
	if (!u)
		return false;
	if (u->statuses.size()!=1)
		return false;
	auto i=u->statuses.find("James K Polk");
	if (i==u->statuses.end())
		return false;
	if (i->second.nick!="James K Polk")
		return false;
	if (i->second.channels.size()!=1)
		return false;
	if (i->second.channels.find("POTUS")==i->second.channels.end())
		return false;
	if (i->second.ip!=QHostAddress("3.4.18.45"))
		return false;
	auto diff=time(NULL)-i->second.lastseen;
	if (diff<10 || diff>12)
		return false;
	this_thread::sleep_for(chrono::minutes(2));
	message mm=p.read();
	while (mm.type!="")
	{
		m=mm;
		mm=p.read();
	}
	if (m.type!="userlist")
		return false;
	u=dynamic_cast<userlist_message *>(m.getdata());
	if (!u)
		return false;
	if (!u->statuses.empty())
		return false;
	return true;
}

void run(plugin_pipe p, string name)
{
	if (name=="userlist")
	{
		p.write(registration_message::create(0, name, "userlist"));
		bool ready=false;
		while (!ready)
		{
			message m=p.blocking_read();
			if (m.type!="registration_status")
				return;
			ready=dynamic_cast<registration_status *>(m.getdata())->status;
		}
		p.write(plugin_adder::create("real_userlist", "lib/libuserlist.so"));
		if (!test_userlist(p))
		{
			p.write(core_quit_message::create());
			return;
		}
	}
	p.write(core_quit_message::create());
	cout << "Success!" << endl;
}
