#ifndef QUIP_MESSAGES_H
#define QUIP_MESSAGES_H

#include <memory>
#include <string>

#include <QString>

// TODO lirch_constants.h.in
#define LIRCH_MESSAGE_TYPE_QUIP_REQUEST "quip_request"

// Encapsulates the data required to send a quip message over the network
class quip_request_data : public message_data
{
public:
	// Allows the core to be responsible with this object's data
	virtual std::unique_ptr<message_data> copy() const
	{
		quip_request_data *raw_copy = new quip_request_data(*this);
		// TODO should this be a pointer to this subtype?
		return std::unique_ptr<message_data>(raw_copy);
	}

	// Allows an easy method for to make message containing this data
	message to_message() const
	{
		std::string message_type = QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST).toStdString();
		quip_request_data *raw_copy = new quip_request_data(*this);
		return message_create(message_type, raw_copy);
	}

	// Request data consist of a destined channel
	quip_request_data(const QString &channel_name) :
		channel(channel_name) { }

	QString channel;
};

#endif // QUIP_MESSAGES_H
