#include <algorithm>
#include <unordered_map>
#include <string>
#include <thread>

#include "message.h"
#include "message_pipe.h"
#include "registry.h"

using namespace std;

static unordered_map<string, thread> plugins;
static unordered_map<string, message_pipe> out_pipes;
static unordered_map<string, registry> message_registrations;
static message_pipe in_pipe;

void to_plugin(message m)
{
	if (message_registrations.count(m.gettype())==0)
		//This message type has no registration, so discard it
		return;
	auto plugin=message_registrations[m.gettype()].get(m.getpriority());
	if (plugin.second=="")
		return;
	if (out_pipes.count(plugin.second)==0)
		//This type does not exist
		return;
	//Change the message priority to the registered priority
	out_pipes[plugin.second].write(m.change_priority(plugin.first));
}

void add_plugin(const message &m)
{
}

void add_registration(const message &m)
{
	//TODO: check if r is of the correct type
	registration_message *r=(registration_message *)m.getdata();

	if (out_pipes.count(r->getname())==0)
		//We can't talk to this plugin, so ignore its request
		return;

	bool b=message_registrations[r->getmessage()].add(r->getpriority(), r->getname());
	//Tell the plugin whether it failed or not
	out_pipes[r->getname()].write(registration_status::create(b, r->getpriority(), r->getmessage()));
}

void process(const message &m)
{
	if (m.gettype()=="add_plugin")
		add_plugin(m);
	else if (m.gettype()=="register")
		add_registration(m);
	else
		to_plugin(m);
}

void run_core(const vector<message> &vm)
{
	for (auto m : vm)
		process(m);
	while (!plugins.empty())
	{
		process(in_pipe.blocking_read());
	}
}

int main()
{
	run_core({});
}
