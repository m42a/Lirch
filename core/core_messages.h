#ifndef CORE_MESSAGES_H_
#define CORE_MESSAGES_H_

#include "message.h"

struct plugin_adder
{
	static constexpr auto message_id="add_plugin";

	std::string name;
	std::string filename;
};

struct targeted_message
{
	static constexpr auto message_id="target";

	std::string name;
	message mess;
};

struct core_quit_message
{
	static constexpr auto message_id="core_quit";
};

#endif
