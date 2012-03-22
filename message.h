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
	message &operator=(const message &m) {type=m.type; priority=m.priority; data=std::move(m.data->copy());}
	message &operator=(message &&) = default;

	//the variables might become private later, so use these functions
	std::string gettype() const {return type;}
	int getpriority() const {return priority;}
	//This gets deleted when the message destructs, so be careful
	message_data *getdata() {return data.get();}

	std::string type;
	int priority;
	std::unique_ptr<message_data> data;
};

class test_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new test_message);}
	static message create(std::string s) {return message(s,0,std::unique_ptr<message_data>(new test_message));}
};

#endif
