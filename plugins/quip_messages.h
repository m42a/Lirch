#include <memory>

#include <QString>

#include "lirch_constants.h"

// Encapsulates the data required to send a quip message over the network
class quip_request_data : public message_data
{
public:
	// Allows the core to be responsible with this object's data
	virtual std::unique_ptr<message_data> copy() const
	{
		quip_request_datum *raw_copy = new quip_request_datum(*this);
		return std::unique_ptr<message_data>(raw_copy);
	}

	// Allows an easy method for to make message containing this data
	message to_message() const
	{
		quip_request_datum *raw_copy = new quip_request_datum(*this);
		return message_create("generate_quip", raw_copy);
	}

	// Con
	quip_request_data(const QString &channel_name) :
		channel(channel_name) { }

	QString channel;
};
