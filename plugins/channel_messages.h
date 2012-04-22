#ifndef CHANNEL_MESSAGES_H_
#define CHANNEL_MESSAGES_H_

class set_channel : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new set_channel(*this));}
	static message create(const QString &ch) {return message_create("set_channel", new set_channel(ch));}

	set_channel(const QString &ch) : channel(ch) {}

	QString channel;
};

class leave_channel : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new leave_channel(*this));}
	static message create(const QString &ch) {return message_create("leave_channel", new leave_channel(ch));}

	leave_channel(const QString &ch) : channel(ch) {}

	QString channel;
};

#endif
