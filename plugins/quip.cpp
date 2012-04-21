#include <cstdio>
#include <unistd.h>

#include "lirch_plugin.h"
#include "grinder_messages.h"

using namespace std;

class generate_quip_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(nullptr);}
	static message create() {return message_create("generate_quip", nullptr);}
};

message generate_generate_quip_message(QString, QString)
{
	return generate_quip_message::create();
}

bool generate_quip(plugin_pipe &pipe)
{
	return true;
}

bool possibly_deal_with_registration_status_message(plugin_pipe &pipe, registration_status *possible_registration_message, const string &plugin_name)
{
	if (possible_registration_message==nullptr)
		return true;
	if (possible_registration_message->status)
		return true;
	if (possible_registration_message->priority>28000)
		pipe.write(registration_message::create(possible_registration_message->priority-1, plugin_name, possible_registration_message->type));
	return true;
}

bool deal_with_message(plugin_pipe &pipe, message m, const string &plugin_name)
{
	if (m.type=="shutdown")
		return false;
	if (m.type=="registration_status")
		return possibly_deal_with_registration_status_message(pipe, dynamic_cast<registration_status *>(m.getdata()), plugin_name);
	if (m.type=="generate_quip")
		return generate_quip(pipe);
	if (m.type=="handler_ready")
		pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	m.decrement_priority();
	pipe.write(m);
	return true;
}

void run(plugin_pipe pipe, string plugin_name)
{
	pipe.write(registration_message::create(30000, plugin_name, "register_handler"));
	pipe.write(registration_message::create(30000, plugin_name, "generate_quip"));
	pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	while (deal_with_message(pipe, pipe.blocking_read(), plugin_name));
	{
	}
}
