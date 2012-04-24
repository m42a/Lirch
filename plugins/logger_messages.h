#ifndef LOGGER_MESSAGES_H
#define LOGGER_MESSAGES_H
#include <map>

#include <QFlags>
#include <QString>

#include "lirch_constants.h"
#include "core/message.h"

// A logging message configures logging of messages
class logging_message : public message_data
{
public:
	enum class logging_format {
		DEFAULT, OFF
	};
	// These allow fields to be optionally set
	// SET_NONE is the default and specifies no changes
	// SET_LDIR specifies changes to the log directory
	// SET_LGBL specifies enabling/disabling logging globally
	// SET_CHAN specifies changes to the channels enabled/disabled
	enum logging_option {
		SET_NONE = 0x0,
		SET_LDIR = 0x1,
		SET_LGBL = 0x2,
		SET_CHAN = 0x4,
	};
	// Generate flags to wrap the enum above
	Q_DECLARE_FLAGS(logging_options, logging_option)

	// Obligatory message copypasta
	virtual std::unique_ptr<message_data> copy() const {
		return std::unique_ptr<message_data>(new logging_message(*this));
	}
	static message create(const logging_message &msg) {
		return message_create(LIRCH_MSG_TYPE_LOGGING, new logging_message(msg));
	}

	// Logging messages should have options specified
	logging_message(logging_options options = logging_options(logging_option::SET_NONE)) :
		log_options(options), log_format(logging_format::DEFAULT) { }

	// Modifiers
	void set_flags(logging_options options) {
		log_options = options;
	}

	void set_directory(const QString &path) {
		log_directory = path;
	}

	void enable_channel(const QString &channel_name) {
		channels[channel_name] = true;
	}

	void disable_channel(const QString &channel_name) {
		channels[channel_name] = false;
	}

	// Accessors
	bool has_option(logging_option option) const {
		return log_options.testFlag(option);
	}

	QString get_directory() const {
		return log_directory;
	}

	std::map<QString, bool>::const_iterator begin() const {
		return channels.begin();
	}

	std::map<QString, bool>::const_iterator end() const {
		return channels.end();
	}

	// Special
	void disable(bool state = true) {
		log_format = (state) ? logging_format::OFF : logging_format::DEFAULT;
	}

	bool is_disabled() const {
		return (log_format == logging_format::OFF);
	}

private:
	logging_options log_options;
	QString log_directory;
	logging_format log_format;
	std::map<QString, bool> channels;
};

// Generate operators
Q_DECLARE_OPERATORS_FOR_FLAGS(logging_message::logging_options)

#endif // LOGGER_MESSAGES_H
