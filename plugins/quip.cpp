#include <cstdio>

#include "lirch_plugin.h"

using namespace std;

message echo_quip()
{
}

bool possibly_deal_with_registration_status_message(plugin_pipe &pipe, registration_message *possible_registration_message, const string &plugin_name)
{
	if (possible_registration_message==nullptr)
		return true;
	if (possible_registration_message->status)
		return true;
	if (possible_registration_message->priority>28000)
		pipe.write(registration_message::create(possible_registration_message->priority-1, plugin_name, possible_registration_message->type));
	return true;
}

bool deal_with_message(plugin_pipe &pipe, message &m, const string &plugin_name)
{
	if (m.type=="shutdown")
		return false;
	if (m.type=="registration_status")
		return possibly_deal_with_registration_status_message(pipe, dynamic_cast<registration_status *>(m.getdata()));
	if (m.type=="handler_ready")
		return false;
	m.decrement_priority();
	pipe.write(m);
	return true;
}

void run(plugin_pipe pipe, string plugin_name)
{
	pipe.write(registration_message::create(30000, plugin_name, "register_handler"));
	pipe.write(register_handler::create("/quip", echo_quip));
	while (deal_with_message(pipe, pipe.blocking_read()));
	{
		pipe.write(register_handler::create("/quip", echo_quip));
	}
}
