#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>
#include <memory>

class message;

//Extend this to make your own message types
class message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const = 0;
	//Derived classes should make static functions to make their message
	//types, but none are declared here because those would necessitate
	//implementation with those specific arguments.
	//static message create_message();
	virtual ~message_data();
protected:
	message_data() = default;
	message_data(const message_data &) = default;
	message_data(message_data &&) = default;
	message_data &operator=(const message_data &) = default;
	message_data &operator=(message_data &&) = default;

};

inline message_data::~message_data() = default;

//These fit into message queues
class message
{
public:
	message() = default;
	message(const message &m) : type(m.type), priority(m.priority), data(m.data->copy()) {}
	message(message &&) = default;
	message(std::string t, int p, std::unique_ptr<message_data> &&d) : type(t), priority(p), data(std::move(d)) {}
	message &operator=(const message &m) {type=m.type; priority=m.priority; data=std::move(m.data->copy()); return *this;}
	message &operator=(message &&) = default;

	//the variables might become private later, so use these functions
	std::string gettype() const {return type;}
	int getpriority() const {return priority;}
	//This gets deleted when the message destructs, so be careful
	message_data *getdata() const {return data.get();}

	static const int initial_priority=0;

	std::string type;
	int priority;
	std::unique_ptr<message_data> data;
};

static message message_create(const std::string &s, message_data *d) {return message(s, message::initial_priority, std::unique_ptr<message_data>(d));}

class test_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new test_message);}
	static message create(const std::string &s) {return message_create(s, new test_message);}
};

class typed_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new typed_message(type, data));}
	static message create(const std::string &t, const std::string &d) {return message_create("typed", new typed_message(t,d));}

	typed_message(const std::string &t, const std::string &d) : type(t), data(d) {}

	std::string type, data;
};

class registration_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new registration_message(priority, plugin_name, message_type));}
	static message create(int p, const std::string &n, const std::string &m) {return message_create("registration", new registration_message(p, n, m));}

	int getpriority() const {return priority;}
	std::string getname() const {return plugin_name;}
	std::string getmessage() const {return message_type;}

	registration_message(int p, const std::string &n, const std::string &m) : priority(p), plugin_name(n), message_type(m) {}

	int priority;
	std::string plugin_name;
	std::string message_type;
};

class registration_status : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new registration_status(status, priority, type));}
	static message create(bool b, int p, const std::string &s) {return message_create("registration_status", new registration_status(b,p,s));}

	registration_status(bool b, int p, const std::string &s) : status(b), priority(p), type(s) {}

	bool status;
	int priority;
	std::string type;
};

#endif
