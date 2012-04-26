#ifndef TIMER_H_
#define TIMER_H_

class timed_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new timed_message(*this));}
	static message create(int m) {return message_create("timer", new timed_message(m));}

	timed_message(int m) : msecs(m) {}

	int msecs;
	message m;
};

#endif
