#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>
#include <memory>
#include <utility>

class message;

class message_data_base
{
public:
	virtual std::unique_ptr<message_data_base> copy() const = 0;


	virtual ~message_data_base();
protected:
	message_data_base() = default;
	message_data_base(const message_data_base &) = default;
	message_data_base(message_data_base &&) = default;
	message_data_base &operator=(const message_data_base &) = default;
	message_data_base &operator=(message_data_base &&) = default;
};

inline message_data_base::~message_data_base() = default;

template <class T>
class message_data : public message_data_base
{
public:
	template <class ...Args>
	message_data(Args &&...args) : t{std::forward<Args>(args)...} {}

	virtual std::unique_ptr<message_data_base> copy() const
	{
		return std::unique_ptr<message_data_base>(new message_data<T>(*this));
	}

	T t;
protected:
	message_data(const message_data &) = default;
	message_data(message_data &&) = default;
	message_data &operator=(const message_data &) = default;
	message_data &operator=(message_data &&) = default;
};

//These fit into message queues
class message
{
public:
	message() = default;
	message(const message &m) : type(m.type), priority(m.priority), data(m.data ? m.data->copy() : nullptr) {}
	message(message &&) = default;
	message(std::string t, int p, std::unique_ptr<message_data_base> &&d) : type(t), priority(p), data(std::move(d)) {}
	message &operator=(const message &m) {type=m.type; priority=m.priority; if (m.data) data=std::move(m.data->copy()); return *this;}
	message &operator=(message &&) = default;

	//the variables might become private later, so use these functions
	std::string gettype() const {return type;}
	int getpriority() const {return priority;}
	message &change_priority(int p) {priority=p; return *this;}
	message &decrement_priority() {--priority; return *this;}
	//This gets deleted when the message destructs, so be careful
	//message_data_base *getdata() const {return data.get();}

	template <class T, class ...Args>
	static message create(Args &&...args)
	{
		return message(T::message_id, initial_priority, std::unique_ptr<message_data_base>(new message_data<T>{std::forward<Args>(args)...}));
	}

	template <class T>
	bool is() const
	{
		return type==T::message_id;
	}

	template <class T>
	T *try_extract() const
	{
		if (is<T>())
			return &(static_cast<message_data<T>*>(data.get())->t);
		return nullptr;
	}

	static constexpr int initial_priority=32767;

	std::string type;
	int priority;
	std::unique_ptr<message_data_base> data;
};

struct empty_message
{
	static constexpr auto message_id="";
};

#endif
