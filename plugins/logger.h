#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>
#include "core/message.h"


class set_logger_directory_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new set_logger_directory_message(*this));}

	static message create(QString dr) {return message_create("set logger directory", new set_logger_directory_message(dr));}

	set_logger_directory_message(QString dr) : directory_root(dr) {}

	QString directory_root;
};

#endif
