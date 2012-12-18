#ifndef REQUIRED_MESSAGES_H_
#define REQUIRED_MESSAGES_H_

#include "message.h"

//Handle these messages

//This message tells you your name when you startup.  It should be the first
//message you receive, and should be sent once.
struct hello_message
{
	static constexpr auto message_id="hello";

	std::string name;
};

//This message tells you to shut your plugin down.
struct shutdown_message
{
	static constexpr auto message_id="shutdown";
};

struct registration_status
{
	static constexpr auto message_id="registration_status";

	bool status;
	int priority;
	std::string type;
};

//You must be able to send these messages

//This message tells the core you are done running.  Your plugin should exit
//immediately after sending this message, but don't call std::exit, since that
//shuts down the whole program; Just return from run.
struct done_message
{
	static constexpr auto message_id="done";

	std::string name;
};

struct registration_message
{
	static constexpr auto message_id="register";

	int priority;
	std::string plugin_name;
	std::string message_type;
};

struct unregistration_message
{
	static constexpr auto message_id="unregister";

	std::string plugin_name;
	std::string message_type;
};

#endif
