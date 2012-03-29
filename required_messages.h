#ifndef REQUIRED_MESSAGES_H_
#define REQUIRED_MESSAGES_H_

#include "message.h"

//Handle these messages

//This message tells you your name when you startup.  It should be the first
//message you receive, and should be sent once.
class hello_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new hello_message(*this));}
	static message create(const std::string &s) {return message_create("hello", new hello_message(s));}

	hello_message(const std::string &s) : name(s) {}

	std::string name;
};

//This message tells you to shut your plugin down.
class shutdown_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new shutdown_message(*this));}
	static message create() {return message_create("shutdown", NULL);}
};

//You must be able to send these messages

//This message tells the core you are done running.  Your plugin should exit
//immediately after sending this message, but don't call std::exit, since that
//shuts down the whole program; Just return from run.
class done_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new done_message(*this));}
	static message create(const std::string &s) {return message_create("done", new done_message(s));}

	done_message(const std::string &n) : name(n) {}

	std::string name;
};

#endif
