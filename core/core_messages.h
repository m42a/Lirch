#ifndef CORE_MESSAGES_H_
#define CORE_MESSAGES_H_

#include "message.h"

class plugin_adder : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new plugin_adder(*this));}
	static message create(const std::string &n, const std::string &f) {return message_create("add_plugin", new plugin_adder(n, f));}

	plugin_adder(const std::string &n, const std::string &f) : name(n), filename(f) {}

	std::string name;
	std::string filename;
};

class targeted_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new targeted_message(*this));}
	static message create(const std::string &n, const message &m) {return message_create("target", new targeted_message(n,m));}

	targeted_message(const std::string &n, const message &m) : name(n), mess(m) {}

	std::string name;
	message mess;
};

class core_quit_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new core_quit_message(*this));}
	static message create() {return message_create("core_quit", NULL);}
};

#endif
