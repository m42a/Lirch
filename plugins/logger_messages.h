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
	// These are expandable for future use
	enum class logging_mode {
		ON, DEFAULT, OFF
	};
	enum class logging_format {
		TXT
	};

	// These allow fields to be optionally set
	// SET_NONE specifies no changes to the log options
	// SET_LDIR specifies changes to the log directory
	// SET_CHAN specifies changes to the channels enabled/disabled
	// SET_MODE specifies enabling/disabling logging globally
	// SET_FORM specifies changing the log format
	enum logging_option {
		SET_NONE = 0x1,
		SET_DIRECTORY = 0x2,
		SET_CHANNELS = 0x4,
		SET_FORMAT = 0x8,
		SET_MODE = 0x10,
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
		log_options(options),
		log_mode(logging_mode::DEFAULT),
		log_format(logging_format::TXT) { }

	// Modifiers
	void set_flags(logging_options options) {
		log_options = options;
	}

	void set_mode(logging_mode mode) {
		log_mode = mode;
	}

	void set_format(logging_format format) {
		log_format = format;
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

	void reset_channel(const QString &channel_name) {
		channels.erase(channel_name);
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

	logging_mode get_mode() const {
		return log_mode;
	}

	logging_format get_format() const {
		return log_format;
	}

private:
	logging_options log_options;
	QString log_directory;
	logging_mode log_mode;
	logging_format log_format;
	std::map<QString, bool> channels;
};

// Generate operators
Q_DECLARE_OPERATORS_FOR_FLAGS(logging_message::logging_options)

#endif // LOGGER_MESSAGES_H
