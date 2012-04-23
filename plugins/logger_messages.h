#ifndef LOGGER_MESSAGES_H
#define LOGGER_MESSAGES_H

#include <QString>

#include "lirch_constants.h"
#include "core/message.h"

class logging_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new logging_message(*this));}

	static message create(QString dr) {return message_create(LIRCH_MSG_TYPE_LOGGING, new logging_message(dr));}

	logging_message(QString dr) : root_directory(dr) {}

	QString root_directory;
};

#endif // LOGGER_MESSAGES_H
