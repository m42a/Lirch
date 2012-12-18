#ifndef QUIP_MESSAGES_H
#define QUIP_MESSAGES_H

// 1) Use only one level of indentation per method
// 2) Don't use the else keyword
// 3) Wrap all primitives and strings
// 4) Use only one dot per line
// 5) Don't abbreviate
// 6) Keep all entities small
// 7) Don't use any classes with more than two instance variables
// 8) Use first-class collections
// 9) Don't use any gettings/setters/properties

#include <memory>
#include <string>

#include <QString>

#include "plugins/quip_constants.h"

// Encapsulates the data required to send a quip message over the network
struct quip_request_data
{
	// Allows an easy method for to make message containing this data
	message to_message() const
	{
		//std::string quip_type = QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST).toStdString();
		//quip_request_data *raw_copy = new quip_request_data(*this);
		return message::create<quip_request_data>(channel);//, raw_copy);
	}

	/*
	// Quip request data consists of a destined channel for a /quip
	quip_request_data(const QString &channel_name) :
		channel(channel_name) { }
	*/

	static constexpr auto message_id=LIRCH_MESSAGE_TYPE_QUIP_REQUEST;

	QString channel;
};

#endif // QUIP_MESSAGES_H
